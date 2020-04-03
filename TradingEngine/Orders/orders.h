#pragma once

#include "MarketOrder.h"
#include "StopLimitOrder.h"

#include <type_traits>

template <typename>
struct IsMarketOrder : std::false_type {};
template <>
struct IsMarketOrder<MarketOrder> : std::true_type {};

template <class T>
inline constexpr bool IsMarketOrder_v = IsMarketOrder<T>::value;

template <typename>
struct IsLimitOrder : std::false_type {};
template <>
struct IsLimitOrder<LimitOrder> : std::true_type {};

template <class T>
inline constexpr bool IsLimitOrder_v = IsLimitOrder<T>::value;

template <typename>
struct IsStopLimitOrder : std::false_type {};
template <>
struct IsStopLimitOrder<StopLimitOrder> : std::true_type {};

template <class T>
inline constexpr bool IsStopLimitOrder_v = IsStopLimitOrder<T>::value;
