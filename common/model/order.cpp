/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 * 
 */

#include "common/model/order.h"

namespace darkpool {

std::ostream& operator<<(std::ostream& os, const Order& o) {
    os << o.to_string();
    return os;
}

bool operator<(const OrderPtr& lhs, const OrderPtr& rhs) {
    return *lhs < * rhs;
}

} // namespace darkpool
