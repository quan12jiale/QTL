#pragma once
#include <QMap>
#include "intervaltreenode.h"

template <class Key, class T>
class IntervalTree
{
public:
	QSet<Interval<Key, T>> all_intervals;
	IntervalTreeNode<Key, T>* top_node;
	QMap<Key, int> boundary_table;

public:
	static IntervalTree* from_tuples(const QList<Key>& begins, const QList<Key>& ends, const QList<T>& datas);
	IntervalTree();
	template <typename Container>
	IntervalTree(const Container& intervals);
	~IntervalTree();
	void _add_boundaries(const Interval<Key, T>& interval);
	void _remove_boundaries(const Interval<Key, T>& interval);

	void add(const Interval<Key, T>& interval);
	void append(const Interval<Key, T>& interval) { this->add(interval); }
	void addi(const Key& begin, const Key& end, const T& data);
	void appendi(const Key& begin, const Key& end, const T& data) { this->addi(begin, end, data); }

	template <typename Container>
	void update(const Container& intervals);
	void remove(const Interval<Key, T>& interval);
	void removei(const Key& begin, const Key& end, const T& data);
	void discard(const Interval<Key, T>& interval);
	void discardi(const Key& begin, const Key& end, const T& data);

	void remove_overlap(const Key& begin, const Key& end);
	void remove_overlap(const Key& begin);
	void remove_envelop(const Key& begin, const Key& end);

	void clear();

	QSet<Interval<Key, T>> at(const Key& p);
	QSet<Interval<Key, T>> envelop(const Key& begin, const Key& end) const;
	QSet<Interval<Key, T>> envelop(const Interval<Key, T>& begin) const;
	QSet<Interval<Key, T>> overlap(const Key& begin, const Key& end) const;
	QSet<Interval<Key, T>> overlap(const Interval<Key, T>& begin) const;

	Key begin() const;
	Key end() const;
	Interval<Key, DummyIntervalData> range() const;
	Key span() const;

	bool __contains__(const Interval<Key, T>& item);
};

template <class Key, class T>
IntervalTree<Key, T>* IntervalTree<Key, T>::from_tuples(const QList<Key>& begins, const QList<Key>& ends, const QList<T>& datas)
{
	if (begins.size() != ends.size() || begins.size() != datas.size())
	{
		return nullptr;
	}

	QList<Interval<Key, T>> ivs;
	ivs.reserve(begins.size());
	for (int i = 0; i < begins.size(); i++)
	{
		ivs.append(Interval<Key, T>(begins[i], ends[i], datas[i]));
	}
	return new IntervalTree<Key, T>(ivs);
}

template <class Key, class T>
IntervalTree<Key, T>::IntervalTree()
{
	this->top_node = nullptr;
	this->clear();
}

template <class Key, class T>
template <typename Container>
IntervalTree<Key, T>::IntervalTree(const Container& intervals)
{
	this->top_node = nullptr;
	this->clear();

	for (const auto& iv : intervals)
	{
		this->all_intervals.insert(iv);
	}
	this->top_node = IntervalTreeNode<Key, T>::from_intervals(this->all_intervals.toList());
	for (const auto& iv : this->all_intervals)
	{
		this->_add_boundaries(iv);
	}
}

template <class Key, class T>
IntervalTree<Key, T>::~IntervalTree()
{
	this->clear();
}

template <class Key, class T>
void IntervalTree<Key, T>::_add_boundaries(const Interval<Key, T>& interval)
{
	Key begin = interval.begin;
	Key end = interval.end;
	if (this->boundary_table.contains(begin))
	{
		this->boundary_table[begin] += 1;
	}
	else
	{
		this->boundary_table[begin] = 1;
	}

	if (this->boundary_table.contains(end))
	{
		this->boundary_table[end] += 1;
	}
	else
	{
		this->boundary_table[end] = 1;
	}
}

template <class Key, class T>
void IntervalTree<Key, T>::_remove_boundaries(const Interval<Key, T>& interval)
{
	Key begin = interval.begin;
	Key end = interval.end;

	int count = this->boundary_table.value(begin, 0);
	if (count == 1)
	{
		this->boundary_table.remove(begin);
	}
	else if (count > 1)
	{
		this->boundary_table[begin] -= 1;
	}

	count = this->boundary_table.value(end, 0);
	if (count == 1)
	{
		this->boundary_table.remove(end);
	}
	else if (count > 1)
	{
		this->boundary_table[end] -= 1;
	}
}

template <class Key, class T>
void IntervalTree<Key, T>::add(const Interval<Key, T>& interval)
{
	if (this->__contains__(interval))
	{
		return;
	}

	if (interval.is_null())
	{
		throw std::exception("ValueError");
	}

	if (!this->top_node)
	{
		this->top_node = IntervalTreeNode<Key, T>::from_interval(interval);
	}
	else
	{
		this->top_node = this->top_node->add(interval);
	}
	this->all_intervals.insert(interval);
	this->_add_boundaries(interval);
}

template <class Key, class T>
void IntervalTree<Key, T>::addi(const Key& begin, const Key& end, const T& data)
{
	this->add(Interval<Key, T>(begin, end, data));
}

/* Container：QList<Interval<Key, T>>、QSet<Interval<Key, T>> */
template <class Key, class T>
template <typename Container>
void IntervalTree<Key, T>::update(const Container& intervals)
{
	for (const auto& iv : intervals)
	{
		this->add(iv);
	}
}

template <class Key, class T>
void IntervalTree<Key, T>::remove(const Interval<Key, T>& interval)
{
	if (!this->__contains__(interval))
	{
		throw std::exception("ValueError");
	}
	this->top_node = this->top_node->remove(interval);
	this->all_intervals.remove(interval);
	this->_remove_boundaries(interval);
}

template <class Key, class T>
void IntervalTree<Key, T>::removei(const Key& begin, const Key& end, const T& data)
{
	this->remove(Interval<Key, T>(begin, end, data));
}

template <class Key, class T>
void IntervalTree<Key, T>::discard(const Interval<Key, T>& interval)
{
	if (!this->__contains__(interval))
	{
		return;
	}
	this->top_node = this->top_node->discard(interval);
	this->all_intervals.remove(interval);
	this->_remove_boundaries(interval);
}

template <class Key, class T>
void IntervalTree<Key, T>::discardi(const Key& begin, const Key& end, const T& data)
{
	this->discard(Interval<Key, T>(begin, end, data));
}

template <class Key, class T>
void IntervalTree<Key, T>::remove_overlap(const Key& begin, const Key& end)
{
	QSet<Interval<Key, T>> hitlist = this->overlap(begin, end);
	for (const auto& iv : hitlist)
	{
		this->remove(iv);
	}
}

template <class Key, class T>
void IntervalTree<Key, T>::remove_overlap(const Key& begin)
{
	QSet<Interval<Key, T>> hitlist = this->at(begin);
	for (const auto& iv : hitlist)
	{
		this->remove(iv);
	}
}

template <class Key, class T>
void IntervalTree<Key, T>::remove_envelop(const Key& begin, const Key& end)
{
	QSet<Interval<Key, T>> hitlist = this->envelop(begin, end);
	for (const auto& iv : hitlist)
	{
		this->remove(iv);
	}
}

template <class Key, class T>
void IntervalTree<Key, T>::clear()
{
	if (this->top_node)
	{
		delete this->top_node;
	}

	this->all_intervals.clear();
	this->top_node = nullptr;
	this->boundary_table.clear();
}

template <class Key, class T>
QSet<Interval<Key, T>> IntervalTree<Key, T>::at(const Key& p)
{
	if (!this->top_node)
	{
		return QSet<Interval<Key, T>>();
	}
	return this->top_node->search_point(p, QSet<Interval<Key, T>>());
}

template <class Key, class T>
QSet<Interval<Key, T>> IntervalTree<Key, T>::envelop(const Key& begin, const Key& end) const
{
	QSet<Interval<Key, T>> result;
	if (!this->top_node)
	{
		return result;
	}
	if (begin >= end)
	{
		return result;
	}
	result = this->top_node->search_point(begin, QSet<Interval<Key, T>>());

	/* std::distance(first, last)返回迭代器之间的距离 */
	int bound_begin = std::distance(this->boundary_table.begin(),
		this->boundary_table.lowerBound(begin));
	int bound_end = std::distance(this->boundary_table.begin(),
		this->boundary_table.lowerBound(end));

	QList<Key> point_list = this->boundary_table.keys().mid(bound_begin, bound_end - bound_begin);
	result += this->top_node->search_overlap(point_list);

	QSet<Interval<Key, T>> res;
	for (const auto& iv : result)
	{
		if (iv.begin >= begin && iv.end <= end)
		{
			res.insert(iv);
		}
	}
	return res;
}

template <class Key, class T>
QSet<Interval<Key, T>> IntervalTree<Key, T>::envelop(const Interval<Key, T>& begin) const
{
	return this->envelop(begin.begin, begin.end);
}

template <class Key, class T>
QSet<Interval<Key, T>> IntervalTree<Key, T>::overlap(const Key& begin, const Key& end) const
{
	QSet<Interval<Key, T>> result;
	if (!this->top_node)
	{
		return result;
	}
	if (begin >= end)
	{
		return result;
	}
	result = this->top_node->search_point(begin, QSet<Interval<Key, T>>());

	int bound_begin = std::distance(this->boundary_table.begin(),
		this->boundary_table.lowerBound(begin));
	int bound_end = std::distance(this->boundary_table.begin(),
		this->boundary_table.lowerBound(end));

	QList<Key> point_list = this->boundary_table.keys().mid(bound_begin, bound_end - bound_begin);
	result += this->top_node->search_overlap(point_list);
	return result;
}

template <class Key, class T>
QSet<Interval<Key, T>> IntervalTree<Key, T>::overlap(const Interval<Key, T>& begin) const
{
	return this->overlap(begin.begin, begin.end);
}

template <class Key, class T>
Key IntervalTree<Key, T>::begin() const
{
	if (this->boundary_table.isEmpty())
	{
		return Key();
	}
	return this->boundary_table.keys()[0];
}

template <class Key, class T>
Key IntervalTree<Key, T>::end() const
{
	if (this->boundary_table.isEmpty())
	{
		return Key();
	}
	return this->boundary_table.keys().last();
}

template <class Key, class T>
Interval<Key, DummyIntervalData> IntervalTree<Key, T>::range() const
{
	return Interval(this->begin(), this->end(), DummyIntervalData());
}

template <class Key, class T>
Key IntervalTree<Key, T>::span() const
{
	if (this->boundary_table.isEmpty())
	{
		return Key();
	}
	return this->end() - this->begin();
}

template <class Key, class T>
bool IntervalTree<Key, T>::__contains__(const Interval<Key, T>& item)
{
	return this->all_intervals.contains(item);
}
