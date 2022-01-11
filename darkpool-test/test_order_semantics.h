/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 *
 */

#ifndef __ORDER_SEMANTICS_HPP__
#define __ORDER_SEMANTICS_HPP__

#include <string>

void testOrderFromString(const std::string& order);
void testOrderCreation();
void testOrderComparison();
void testOrderHeapAllocation(int count);
void testOrderGenerator(int count, int num_instruments);
void testOrderRanking();
void testOrderHeapOrdering();
void testIntegerHistograms();
void testDoubleHistograms();

#endif