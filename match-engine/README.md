# Dark Pool Matching Engine

## Requirements

Your application is a matching engine that receives orders and continuously matches them to see if they can trade. Orders are timestamped by their arrival time. Orders are first ranked according to their price; orders of the same price are then ranked depending on the timestamp at entry. Read orders from an input file using the format below and emit the trades. Print all remaining orders that didn’t fully trade after all input is read. Please note the streaming nature of this input and structure your program accordingly.

Input format:

Every line of input contains an order. The order consists of an OrderID, Side, Instrument, Qty and Price. The orders are listed in same order as the time of entry.

Output format:

Please output any trades that are possible in the following format:

* The word TRADE followed by Instrument, OrderID, ContraOrderID, Qty and Price.
* At the end of reading input, please list all remaining orders in the same format as the entry: 
    OrderID, Side, Instrument, RemainingQty, Price.

 Sample Input

     12345 BUY BTCUSD   5   10000
     zod42 SELL BTCUSD   2   10001
     13471 BUY BTCUSD   6   9971
     11431 BUY ETHUSD   9   175
     abe14 SELL BTCUSD   7   9800
     plu401 SELL ETHUSD   5   170
     45691 BUY ETHUSD   3    180
    
 Sample Output

     TRADE BTCUSD abe14 12345 5 10000
     TRADE BTCUSD abe14 13471 2 9971
     TRADE ETHUSD plu401 11431 5 175

     zod42 SELL BTCUSD 2 10001
     13471 BUY BTCUSD 4 9971
     11431 BUY ETHUSD 4 175
     45691 BUY ETHUSD 3 180

## Design

### Order input and boilerplate

* **main.cc** reads command line and creates **OrderProvider**, either **FilenameStreamOrderProvider** or **StdinOrderProvider**
* **main.cc** Creates the main service that drives program: **DarkPoolService**
* **DarkPoolService** defines the order routing strategy. NOTE: DarkPool service would likely read from a config file and determine strategies to use. Currently they are hard-coded, but the flexibility is in the simple class hierarchy.
* The **OrderRouter** invokes get_orders() on **OrderProvider** uses the **RoutingStrategy** to create the **MatchingEngines** to use. I provide one implementation of **RoutingStrategy** which is **InstrumentBoundaryRoutingStrategy** which is a simple class that creates a number of **MatchingEngines** based on a vector with symbol boundaries, such as ["M", "Z"]. The semantics are slightly awkward as:
  * The nth partition is defined as < lexicographically as per std::string operator< and the last partition is ignored as it gets all of the remaining orders. To hide the awkwardness of this simple routing strategy, you can simply specify concurrency and symbol split will be auto-configured.

### Matching Engine

The **MatchingEngine** creates one order book per instrument. Each order book is an instance of **OrderBook**, which creates *priority queues* (heaps) separately each for buys and sells (when we get a buy order we only match against sells). The orders are ranked as per requirements, by price and then by arrival time. Ranking is done by operator< overload for **Order** class.

Why did I choose a *priority queue*?

* I wanted viable contra check to be fast. If the first order in the queeue doesn't match on price, none of them will so we can ignore the trade possibility immediately.
* Heaps are also minimally/partially sorted so they are efficient at line rate since they don't perform a full sort. However, we use a binary vector as a backing store for the heap and insertion can be O(log n) and we'll hit that a lot. My observation is still that it's very fast. A possible optimization would be to use a binomial heap, which can have O(1) insertion time.

## Open Issue

I did not write code to synchronize TRADE output. Therefore when you use concurrency > 1, you may get some wonky TRADE output like:

    TRADE atPR1 UmPOF wwEMQ 4 5679
    TRADE atPR1 UmPOF KDXKV TRADE 5 sbzay ewGXk TftIe 6 5678
    TRADE bO4bN g3HgQ QEEY7 2 5342
    TRADE bO4bN g3HgQ qXO9G 1 5342
    TRADE e4poS FXb4q 5nLsq 4 165
    TRADE e4poS 7dfER 5nLsq 1 165
    TRADE e4poS 7dfER FkVFM 1 165
    7480
    TRADE zLOZ2 sBjOM zCj0i 2 2430

I am happy to fix this limitation is you feel it was a major point of evaluation. Even though Concurrency == 1 has two threads (input + matching engine), this setting won't exhibit the problem since there is only one MatchingEngine generating trades.

## Testing

### Basic Use Cases

You can generate data and send it to the matching engine like so:

    build/DarkPoolTest testOrderGenerator 10000 | build/DarkPool

I created test files like so:

    build/DarkPoolTest testOrderGenerator 1000 > sample-input/order-1k.txt
    build/DarkPoolTest testOrderGenerator 1000000 > sample-input/order-1m.txt
    build/DarkPoolTest testOrderGenerator 100000000 > sample-input/order-100m.txt

You can also control the rate with a little bash magic in the middle:

    build/DarkPoolTest testOrderGenerator 10000 | while read line; do echo $line; sleep 0.1; done | build/DarkPool

### Checking for memory leaks

Pump in a million orders and check for leaks:

    build/DarkPoolTest testOrderGenerator 1000000 > sample-input/order-1m.txt
    valgrind -s --leak-check=yes --leak-check=full --show-leak-kinds=all --track-origins=yes build/DarkPool sample-input/order-1m.txt > /dev/null

Results:

    ==32127== HEAP SUMMARY:
    ==32127==     in use at exit: 0 bytes in 0 blocks
    ==32127==   total heap usage: 2 allocs, 2 frees, 76,800 bytes allocated
    ==32127==
    ==32127== All heap blocks were freed -- no leaks are possible

### Benchmarks

Concurrency does not always benefit overall performance in my implementation. Before I made batching enhancements, I noticed considerable overhead in synchronizing threads for most smaller to medium workloads. If the match function is fast and is acting on small to medium order books, the lock contention between the input thread locking per-order against the MatchingEngine threads pulling orders, lock contention reduces performance (considerably). This is a well-known problem for naive implementations like mine.

However, in an effort to get some benefit from parallelization, I added a "batching" feature to OrderProvider. Instead of providing one order at a time, OrderProvider interface can forward a vector (batch) of orders. Batching is controlled by the second command-line parameter.

    build/DarkPool <filename> batch-size concurrency
    build/DarkPool - batch-size concurrency  # to read from stdin

Doing this vastly reduced the calls to lock on the input thread and improved performance (see below).

The third parameter, concurrency, dictates how many MatchingEngines are allocated. Workloads are scaled out by instrument name using **InstrumentBoundaryRoutingStrategy**. When concurency > 0, we have concurrency threads each managing one Matching Engine. Each of these MatchingEngine maintains order books for a range of instruments.

Here is a basic performance table, balancing batch size and concurrency for a 1m row input. This command is used:

    $ time build/DarkPool sample-input/order-1m.txt 1 0 > /dev/null

Results:

                      Batch Size
                      1           10          100         1,000
                 ---+---------------------------------------------
    Concurrency  0  | 0m2.178s    0m1.871s    0m1.831s    0m1.787s
                 1  | 0m1.238s    0m1.059s    0m1.001s    0m1.007s
                 2  | 0m1.299s    0m1.109s    0m1.110s    0m1.044s
                 4  | 0m2.017s    0m1.934s    0m1.900s    0m1.954s
                 8  | 0m2.596s    0m2.560s    0m2.542s    0m2.487s
                 16 | 0m3.092s    0m2.993s    0m2.963s    0m3.043s

Observe:

* Best performance was with 1 worker thread at 100 batch.
* Concurrency beyond 2 has no benefit for small batch sizes
* Even with batching at 100, best performance was seen with a single worker thread separate from order read thread.
* This could be something wrong with the code as you might expect concurrency to provide more benefit. See below.

To consider the overhead of I/O, I disabled TRADE and unexecuted orders from being output to console. Using this command (notice no redirection):

    $ time build/DarkPool sample-input/order-1m.txt x y

Results:

                      Batch Size
                      1           10          100         1,000      10,000    100,000
                 ---+-----------------------------------------------------------------
    Concurrency  0  | 0m1.331s    0m1.156s    0m1.098s    0m1.073s
                 1  | 0m1.354s    0m1.002s    0m0.889s    0m0.875s   0m0.895s  0m0.950s
                 2  | 0m1.374s    0m1.058s    0m0.933s    0m0.933s   0m0.900s  0m0.930s
                 4  | 0m2.494s    0m1.697s    0m1.227s    0m1.046s   0m0.898s  0m0.972s
                 8  | 0m2.941s    0m2.002s    0m1.576s    0m1.172s   0m0.996s  0m1.070s
                 16 | 0m2.748s    0m3.193s    0m2.119s    0m1.628s   0m1.397s  0m1.519s

Observe:

* Best performance without stdout I/O was with 1 worker thread and 100 batch.
* The whole point of batching is to give the async threads something meaty to do so they are not starved waiting for work.
* We do this at the _expense of latency_ as now we're not reacting to new orders as fast as possible, they are batched first.

Alternatively, if we insert an artificial delay into the match() function, say 10ms, we can see parallelism improves more drastically.

We use 1k orders instead of 1m:

    $ time build/DarkPool sample-input/order-1m.txt x y

Results:

                      Batch Size
                      1           10          100     
                 ---+---------------------------------
    Concurrency  0  | 0m9.084s    0m9.081s    0m9.080s (no concurrency)
                 1  | 0m9.078s    0m9.076s    0m9.080s (one input threads, one matching thread)
                 2  | 0m5.054s    0m5.055s    0m5.054s
                 4  | 0m3.080s    0m3.080s    0m3.078s
                 8  | 0m2.372s    0m2.372s    0m2.374s
                 16 | 0m3.080s    0m3.079s    0m3.079s

Observe:

* With the artificial delay, best performance was with 8 threads, batching has no effect (and can only hurt latency reporting for trades)
* In these cases we can see that batching has absolutely no benefit as the match() task already takes long enough to see benefits with parallelism. In fact, parallelism drops between 8 and 16 threads.

### Detailed analysis stats

When the DarkPool is run, detailed statistics are generated in *.bins and *.histogram files. Each file is explained:


    Filename                                            Content
    -----------------------------------------------------------------------------------------------------------------
    InputThreadLockLatenciesMicros.bins                 Total micros *input thread* waiting for mutex each second
    InputThreadLockLatenciesMicros.histogram            Percentiles in micros for *input thread* waiting for mutex overall

    MatchingEngine-Lock-LatenciesMicros-U.bins          Total micros *MatchingEngine U* is waiting for mutex each second
    MatchingEngine-Lock-LatenciesMicros-U.histogram     Percentiles in micros for *MatchingEngine U* waiting for mutex overall
    MatchingEngine-Lock-LatenciesMicros-z.bins          
    MatchingEngine-Lock-LatenciesMicros-z.histogram     
    
    MatchingEngine-match-LatenciesMicros-U.bins         Total micros *MatchingEngine U* is spends in match() function
    MatchingEngine-match-LatenciesMicros-U.histogram    Percentiles in micros *MatchingEngine U* spends in match() function
    MatchingEngine-match-LatenciesMicros-z.bins         
    MatchingEngine-match-LatenciesMicros-z.histogram    

    MatchingEngine-OrderBookSizes-U.bins                Average order book sizes each second (sum order sizes / number of books)
    MatchingEngine-OrderBookSizes-z.bins
    
    MatchingEngine-OrdersProcessed-U.bins               Orders processed per second
    MatchingEngine-OrdersProcessed-z.bins

Using these statistics, we can tune batch sizes and concurrency and optimize the code. I did not exhaustively use these stats to come up with optimal parameters as  left that up to discussion.

### Input Data and Effect on Performance

The kind of input can also affect. The test generator program generates a nearly random and nearly normal distribution of symbols. Same goes for prices around what we generate as a "last" price in a pseudo security master. The generator generates X symbols initially and chooses a "last price" and stores this in a collection of Instrument objects. The generator then chooses amongst these symbols only and generates a random price clustered around this last price. This way we get a good set of BUY/SELL limit orders where some will execute and some will lay unexecuted in the order book. If the data is perfect and generates TRADE records at high frequency, remaining orders in order book will stay small and inserts into the priority queue will be extremely fast. As we get more orders that don't create trades, and order book sizes growm inserts into the order book may take longer.

I didn't do benchmarks against a _degenerate_ order book where, say, nothing matches. This could be the case in a sinking (or, conversely, rising) market where the herd mentality causes most traders to BUY and SELL orders are scarce (or vice versa).

### Benchmarks Conclusion and Possible Optimizations

My gut says the priority queue did a good job with this problem but my threading model could use work. It's well known that locks between threads is sub-optimal in this day and age, but when I considered implementing my own lock-free queue (like LMAX disrupter), I felt like that would be a whole project itself. A lock-free queue woudl be a bounded queue usually implemented as a ring buffer. The readers read from one end and the writers read from another and there are a few atomic and lock-free checks that can be done to make sure the counters don't run into one another but there is no locking. If there is a fast writer, additional linked queues are allocated so no locking is necessary.

Another possible optimization would be to pin threads to individual cores to maximize cache re-use on the dedicated CPU, this could help with the match() function and priority-queue rebalace operations since they would probably all fit in the fast L1-L3 caches. There is an API in linux to set thread affinity, pthread_setaffinity_np. Again, I could spend more time on this to optimize further.

## Appendix

## CPU for Benchmarking

AMD Ryzen 9 3900X 12-Core Processor (24 threads) running on **WSL2/Windows 10**.

    $ lscpu
    Architecture:                    x86_64
    CPU op-mode(s):                  32-bit, 64-bit
    Byte Order:                      Little Endian
    Address sizes:                   48 bits physical, 48 bits virtual
    CPU(s):                          24
    On-line CPU(s) list:             0-23
    Thread(s) per core:              2
    Core(s) per socket:              12
    Socket(s):                       1
    Vendor ID:                       AuthenticAMD
    CPU family:                      23
    Model:                           113
    Model name:                      AMD Ryzen 9 3900X 12-Core Processor
    Stepping:                        0
    CPU MHz:                         3792.902
    BogoMIPS:                        7585.80
    Hypervisor vendor:               Microsoft
    Virtualization type:             full
    L1d cache:                       384 KiB
    L1i cache:                       384 KiB
    L2 cache:                        6 MiB
    L3 cache:                        16 MiB
