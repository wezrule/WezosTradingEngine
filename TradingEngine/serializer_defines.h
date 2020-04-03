#pragma once

// Helper MACROS to separate the actual archiving of classes to somewhere else
// (they make the serializing class a friend of the class being archived).
#define SERIALIZE_HEADER(A)                                      \
	class A;                                                       \
	namespace boost::serialization {                               \
	template <class Archive>                                       \
	void serialize(Archive& ar, A& g, const unsigned int version); \
	}

#define SERIALIZE_FRIEND(A) \
	template <class Archive>  \
	friend void boost::serialization::serialize(Archive& ar, A& g, const unsigned int version);
