#pragma once

behavior OrderBookActor {

}

behavior OrderBookActor(event_based_actor* self) {
  return {
    [=](const std::string& order) { aout(self) << "Received order: " << order << std::endl; },
  };
}