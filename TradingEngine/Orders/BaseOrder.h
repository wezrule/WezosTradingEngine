#pragma once

#include "../serializer_defines.h"

#include <cstdint>

SERIALIZE_HEADER(BaseOrder)

class BaseOrder {
public:
	SERIALIZE_FRIEND(BaseOrder)

	BaseOrder() = default;
	int64_t GetAmount() const;
	int32_t GetUserId() const;
	int64_t GetRemaining() const;
	int64_t GetFilled() const;
	void AddToFill(int64_t fill);
	void RemoveFromFill(int64_t fill);
	void SetId(int64_t orderId);
	int64_t GetId() const;
	bool operator==(const BaseOrder& order) const;

protected:
	BaseOrder(int32_t userId, int64_t amount, int64_t filled);

private:
	int64_t orderId = 0;
	int32_t userId;
	int64_t amount;
	int64_t filled;
};
