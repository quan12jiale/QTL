#pragma once
#include <QSet>
#include <QtCore/qtypetraits.h>

template <class Key>
static inline bool myFuzzyCompare(const Key& p1, const Key& p2)
{
	return qAbs(p1 - p2) * 1000000. <= 1.;
}

struct DummyIntervalData
{
	bool operator==(const DummyIntervalData&) const { return true; }
	bool operator<(const DummyIntervalData&) const { return false; }
};

/*
Key可以是浮点数、整数、以及其它支持小于、大于、等于、小于等于、大于等于运算符、qHash的类型，如CVector2d
T可以是支持等于、小于的类型
*/
template <class Key, class T>
class Interval
{
public:
	Key begin;
	Key end;
	T data;

public:
	Interval(const Key& begin, const Key& end, const T& data);
	bool overlaps(const Key& begin, const Key& end) const;
	bool overlaps(const Key& begin) const;
	bool overlaps(const Interval& begin) const;

	bool contains_point(const Key& p) const;
	bool range_matches(const Interval& other) const;
	bool contains_interval(const Interval& other) const;

	Key distance_to(const Interval& other) const;
	bool is_null() const;
	Key length() const;

	// #注意: qHash函数需要在.h文件中的类中实现
	friend uint qHash(const Interval<Key, T>& key, uint seed)
	{
		return qHash(key.begin, seed) ^ qHash(key.end, seed);
	}
	bool operator==(const Interval& other) const;

	int __cmp__(const Interval& other) const;
	bool operator<(const Interval& other) const;
	bool operator>(const Interval& other) const;
};

template <class Key, class T>
Interval<Key, T>::Interval(const Key& begin, const Key& end, const T& data)
{
	this->begin = begin;
	this->end = end;
	this->data = data;
}

template <class Key, class T>
bool Interval<Key, T>::overlaps(const Key& begin, const Key& end) const
{
	return begin < this->end && end > this->begin;
}

template <class Key, class T>
bool Interval<Key, T>::overlaps(const Key& begin) const
{
	return this->contains_point(begin);
}

template <class Key, class T>
bool Interval<Key, T>::overlaps(const Interval& begin) const
{
	return this->overlaps(begin.begin, begin.end);
}

template <class Key, class T>
bool Interval<Key, T>::contains_point(const Key& p) const
{
	return this->begin <= p && p < this->end;
}

template <class Key, class T>
bool Interval<Key, T>::range_matches(const Interval<Key, T>& other) const
{
	if (QtPrivate::is_floating_point<Key>::value) // 如果Key是浮点数类型
	{
		return myFuzzyCompare(this->begin, other.begin) && myFuzzyCompare(this->end, other.end);
	}
	else // if (QtPrivate::is_integral<Key>::value) // 如果Key是整数类型
	{
		return this->begin == other.begin && this->end == other.end;
	}
}

template <class Key, class T>
bool Interval<Key, T>::contains_interval(const Interval& other) const
{
	return this->begin <= other.begin && this->end >= other.end;
}

template <class Key, class T>
Key Interval<Key, T>::distance_to(const Interval& other) const
{
	if (this->overlaps(other))
	{
		return 0;
	}
	if (this->begin < other.begin)
		return other.begin - this->end;
	else
		return this->begin - other.end;
}

template <class Key, class T>
bool Interval<Key, T>::is_null() const
{
	return this->begin > this->end;
}

template <class Key, class T>
Key Interval<Key, T>::length() const
{
	if (this->is_null())
	{
		return 0;
	}
	return this->end - this->begin;
}

template <class Key, class T>
bool Interval<Key, T>::operator==(const Interval& other) const
{
	return this->range_matches(other) && this->data == other.data;
}

template <class Key, class T>
int Interval<Key, T>::__cmp__(const Interval& other) const
{
	if (QtPrivate::is_floating_point<Key>::value) // 如果Key是浮点数类型
	{
		if (!myFuzzyCompare(this->begin, other.begin))
		{
			return this->begin < other.begin ? -1 : 1;
		}
		if (!myFuzzyCompare(this->end, other.end))
		{
			return this->end < other.end ? -1 : 1;
		}
	}
	else // if (QtPrivate::is_integral<Key>::value) // 如果Key是整数类型
	{
		if (this->begin != other.begin)
		{
			return this->begin < other.begin ? -1 : 1;
		}
		if (this->end != other.end)
		{
			return this->end < other.end ? -1 : 1;
		}
	}
	if (this->data == other.data)
	{
		return 0;
	}
	return this->data < other.data ? -1 : 1;
}

template <class Key, class T>
bool Interval<Key, T>::operator<(const Interval& other) const
{
	return this->__cmp__(other) < 0;
}

template <class Key, class T>
bool Interval<Key, T>::operator>(const Interval& other) const
{
	return this->__cmp__(other) > 0;
}

