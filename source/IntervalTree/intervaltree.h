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
	IntervalTree(const QSet<Interval<Key, T>>& intervals = QSet<Interval<Key, T>>());
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

	QSet<Interval<Key, T>> envelop(const Key& begin, const Key& end) const;
	QSet<Interval<Key, T>> envelop(const Interval<Key, T>& begin) const;

	bool __contains__(const Interval<Key, T>& item);
};



template <class Key, class T>
IntervalTree<Key, T>* IntervalTree<Key, T>::from_tuples(const QList<Key>& begins, const QList<Key>& ends, const QList<T>& datas)
{
	if (begins.size() != ends.size() || begins.size() != datas.size())
	{
		return nullptr;
	}
	QSet<Interval<Key, T>> ivs;
	for (int i = 0; i < begins.size(); i++)
	{
		ivs.insert(Interval<Key, T>(begins[i], ends[i], datas[i]));
	}
	return new IntervalTree<Key, T>(ivs);
}

template <class Key, class T>
IntervalTree<Key, T>::IntervalTree(const QSet<Interval<Key, T>>& intervals)
{
	for (const auto& iv : intervals)
	{
		if (iv.is_null())
		{
			throw std::exception("ValueError");
		}
	}

	this->all_intervals = intervals;
	this->top_node = IntervalTreeNode<Key, T>::from_intervals(this->all_intervals.toList());
	this->boundary_table.clear();
	for (const auto& iv : this->all_intervals)
	{
		this->_add_boundaries(iv);
	}
}

template <class Key, class T>
IntervalTree<Key, T>::~IntervalTree()
{
	if (this->top_node)
	{
		delete this->top_node;
	}
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
QSet<Interval<Key, T>> IntervalTree<Key, T>::envelop(const Key& begin, const Key& end) const
{
	QSet<Interval<Key, T>> result;
	IntervalTreeNode<Key, T>* root = this->top_node;
	if (!root)
	{
		return result;
	}
	if (begin >= end)
	{
		return result;
	}
	result = root->search_point(begin, QSet<Interval<Key, T>>());

	// std::distance(first, last)返回迭代器之间的距离
	int bound_begin = std::distance(this->boundary_table.begin(),
		this->boundary_table.lowerBound(begin));
	int bound_end = std::distance(this->boundary_table.begin(),
		this->boundary_table.lowerBound(end));

	QList<Key> point_list = this->boundary_table.keys().mid(bound_begin, bound_end - bound_begin);
	QSet<Interval<Key, T>> tmp_set = root->search_overlap(point_list);
	result += tmp_set;

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
bool IntervalTree<Key, T>::__contains__(const Interval<Key, T>& item)
{
	return this->all_intervals.contains(item);
}