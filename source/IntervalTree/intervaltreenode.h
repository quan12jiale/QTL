#pragma once
#include <functional>
#include "interval.h"

template <class Key, class T>
static bool sortByEndBegin(const Interval<Key, T>& iv1, const Interval<Key, T>& iv2)
{
	if (QtPrivate::is_floating_point<Key>::value) // 如果Key是浮点数类型
	{
		if (!myFuzzyCompare(iv1.end, iv2.end))
		{
			return iv1.end < iv2.end;
		}
		return iv1.begin < iv2.begin;
	}
	else
	{
		if (iv1.end != iv2.end)
		{
			return iv1.end < iv2.end;
		}
		return iv1.begin < iv2.begin;
	}
}


template <class Key, class T>
class IntervalTreeNode
{
public:
	Key x_center;
	QSet<Interval<Key, T>> s_center;
	IntervalTreeNode* left_node;
	IntervalTreeNode* right_node;
	int depth;
	int balance;

public:
	IntervalTreeNode(const Key& x_center = Key(),
		const QList<Interval<Key, T>>& s_center = QList<Interval<Key, T>>(),
		IntervalTreeNode* left_node = nullptr,
		IntervalTreeNode* right_node = nullptr);
	~IntervalTreeNode();
	static IntervalTreeNode* from_interval(const Interval<Key, T>& interval);
	static IntervalTreeNode* from_intervals(QList<Interval<Key, T>>& intervals);
	IntervalTreeNode* init_from_sorted(const QList<Interval<Key, T>>& intervals);

	bool center_hit(const Interval<Key, T>& interval) const;
	bool hit_branch(const Interval<Key, T>& interval) const;
	void refresh_balance();
	int compute_depth() const;

	IntervalTreeNode* rotate();
	IntervalTreeNode* srotate();
	IntervalTreeNode* drotate();

	IntervalTreeNode* add(const Interval<Key, T>& interval);
	IntervalTreeNode* remove(const Interval<Key, T>& interval);
	IntervalTreeNode* discard(const Interval<Key, T>& interval);
	IntervalTreeNode* remove_interval_helper(const Interval<Key, T>& interval, 
		QList<int>& done, bool should_raise_error);

	QSet<Interval<Key, T>> search_overlap(const QList<Key>& point_list);
	QSet<Interval<Key, T>> search_point(const Key& point, QSet<Interval<Key, T>>& result);
	IntervalTreeNode* prune();
	std::pair<IntervalTreeNode*, IntervalTreeNode*> pop_greatest_child();
	bool contains_point(const Key& p);
	QSet<Interval<Key, T>> all_children();
	QSet<Interval<Key, T>> all_children_helper(QSet<Interval<Key, T>>& result);
	void verify(const QSet<Key>& parents);

	IntervalTreeNode* &at(bool index);
	QString str() const;
	int count_nodes() const;
	double depth_score(int n, int m) const;
	int depth_score_helper(int d, int dopt) const;

	//QString print_structure(int indent = 0) const;
};

/* Container：QList<Interval<Key, T>>、QSet<Interval<Key, T>> */
template <class Key, class T>
IntervalTreeNode<Key, T>::IntervalTreeNode(const Key& x_center,
	const QList<Interval<Key, T>>& s_center,
	IntervalTreeNode* left_node,
	IntervalTreeNode* right_node)
{
	this->x_center = x_center;
	this->s_center = s_center.toSet();
	this->left_node = left_node;
	this->right_node = right_node;
	this->depth = 0;
	this->balance = 0;
	this->rotate();
}

template <class Key, class T>
IntervalTreeNode<Key, T>::~IntervalTreeNode()
{
	if (this->left_node)
	{
		delete this->left_node;
	}
	if (this->right_node)
	{
		delete this->right_node;
	}
}

template <class Key, class T>
IntervalTreeNode<Key, T>* IntervalTreeNode<Key, T>::from_interval(const Interval<Key, T>& interval)
{
	Key center = interval.begin;
	QList<Interval<Key, T>> s_center_list;
	s_center_list.append(interval);
	return new IntervalTreeNode<Key, T>(center, s_center_list);
}

template <class Key, class T>
IntervalTreeNode<Key, T>* IntervalTreeNode<Key, T>::from_intervals(QList<Interval<Key, T>>& intervals)
{
	if (intervals.isEmpty())
	{
		return nullptr;
	}
	IntervalTreeNode<Key, T>* node = new IntervalTreeNode<Key, T>;
	std::sort(intervals.begin(), intervals.end());
	node = node->init_from_sorted(intervals);
	return node;
}

template <class Key, class T>
IntervalTreeNode<Key, T>* IntervalTreeNode<Key, T>::init_from_sorted(const QList<Interval<Key, T>>& intervals)
{
	Interval<Key, T> center_iv = intervals[intervals.size() / 2];
	this->x_center = center_iv.begin;
	this->s_center.clear();
	QList<Interval<Key, T>> s_left, s_right;
	for (const auto& k : intervals)
	{
		if (k.end <= this->x_center)
		{
			s_left.append(k);
		}
		else if (k.begin > this->x_center)
		{
			s_right.append(k);
		}
		else
		{
			this->s_center.insert(k);
		}
	}
	this->left_node = IntervalTreeNode::from_intervals(s_left);
	this->right_node = IntervalTreeNode::from_intervals(s_right);
	return this->rotate();
}

template <class Key, class T>
bool IntervalTreeNode<Key, T>::center_hit(const Interval<Key, T>& interval) const
{
	return interval.contains_point(this->x_center);
}

template <class Key, class T>
bool IntervalTreeNode<Key, T>::hit_branch(const Interval<Key, T>& interval) const
{
	return interval.begin > this->x_center;
}

template <class Key, class T>
void IntervalTreeNode<Key, T>::refresh_balance()
{
	int left_depth = this->left_node ? this->left_node->depth : 0;
	int right_depth = this->right_node ? this->right_node->depth : 0;
	this->depth = 1 + std::max(left_depth, right_depth);
	this->balance = right_depth - left_depth;
}

template <class Key, class T>
int IntervalTreeNode<Key, T>::compute_depth() const
{
	int left_depth = this->left_node ? this->left_node->compute_depth() : 0;
	int right_depth = this->right_node ? this->right_node->compute_depth() : 0;
	return 1 + std::max(left_depth, right_depth);
}

template <class Key, class T>
IntervalTreeNode<Key, T>* IntervalTreeNode<Key, T>::rotate()
{
	this->refresh_balance();
	if (std::abs(this->balance) < 2)
		return this;
	bool my_heavy = this->balance > 0;
	bool child_heavy = this->at(my_heavy)->balance > 0;
	if (my_heavy == child_heavy || this->at(my_heavy)->balance == 0)
	{
		return this->srotate();
	}
	else
	{
		return this->drotate();
	}
}

template <class Key, class T>
IntervalTreeNode<Key, T>* IntervalTreeNode<Key, T>::srotate()
{
	bool heavy = this->balance > 0;
	bool light = !heavy;
	IntervalTreeNode<Key, T>* save = this->at(heavy);
	this->at(heavy) = save->at(light);
	save->at(light) = this->rotate();

	QList<Interval<Key, T>> promotees;
	QSet<Interval<Key, T>> save_s_center = save->at(light)->s_center;
	for (auto const& iv : save_s_center)
	{
		if (save->center_hit(iv))
		{
			promotees.append(iv);
		}
	}
	if (!promotees.isEmpty())
	{
		for (auto const& iv : promotees)
		{
			save->at(light) = save->at(light)->remove(iv);
		}
		save->s_center += promotees.toSet();
	}
	save->refresh_balance();
	return save;
}

template <class Key, class T>
IntervalTreeNode<Key, T>* IntervalTreeNode<Key, T>::drotate()
{
	bool my_heavy = this->balance > 0;
	this->at(my_heavy) = this->at(my_heavy)->srotate();
	this->refresh_balance();
	IntervalTreeNode<Key, T>* result = this->srotate();
	return result;
}

template <class Key, class T>
IntervalTreeNode<Key, T>* IntervalTreeNode<Key, T>::add(const Interval<Key, T>& interval)
{
	if (this->center_hit(interval))
	{
		this->s_center.insert(interval);
		return this;
	}
	else
	{
		bool direction = this->hit_branch(interval);
		if (!this->at(direction))
		{
			this->at(direction) = IntervalTreeNode<Key, T>::from_interval(interval);
			this->refresh_balance();
			return this;
		}
		else
		{
			this->at(direction) = this->at(direction)->add(interval);
			return this->rotate();
		}
	}
}

template <class Key, class T>
IntervalTreeNode<Key, T>* IntervalTreeNode<Key, T>::remove(const Interval<Key, T>& interval)
{
	QList<int> done;
	return this->remove_interval_helper(interval, done, true);
}

template <class Key, class T>
IntervalTreeNode<Key, T>* IntervalTreeNode<Key, T>::discard(const Interval<Key, T>& interval)
{
	QList<int> done;
	return this->remove_interval_helper(interval, done, false);
}

template <class Key, class T>
IntervalTreeNode<Key, T>* IntervalTreeNode<Key, T>::remove_interval_helper(const Interval<Key, T>& interval,
	QList<int>& done, bool should_raise_error)
{
	if (this->center_hit(interval))
	{
		if (!should_raise_error && !this->s_center.contains(interval))
		{
			done.append(1);
			return this;
		}
		if (this->s_center.contains(interval))
		{
			this->s_center.remove(interval);
		}
		else
		{
			// QString structure = this->print_structure();
			throw std::exception("KeyError");
		}
		if (!this->s_center.isEmpty())
		{
			done.append(1);
			return this;
		}
		return this->prune();
	}
	else
	{
		bool direction = this->hit_branch(interval);

		if (!this->at(direction))
		{
			if (should_raise_error)
			{
				throw std::exception("ValueError");
			}
			done.append(1);
			return this;
		}

		this->at(direction) = this->at(direction)->remove_interval_helper(interval, done, should_raise_error);

		if (done.isEmpty())
		{
			return this->rotate();
		}
		return this;
	}
}

template <class Key, class T>
QSet<Interval<Key, T>> IntervalTreeNode<Key, T>::search_overlap(const QList<Key>& point_list)
{
	QSet<Interval<Key, T>> result;
	for (const auto& j : point_list)
	{
		this->search_point(j, result);
	}
	return result;
}

template <class Key, class T>
QSet<Interval<Key, T>> IntervalTreeNode<Key, T>::search_point(const Key& point, QSet<Interval<Key, T>>& result)
{
	for (const auto& k : this->s_center)
	{
		if (k.begin <= point && point < k.end)
		{
			result.insert(k);
		}
	}
	if (point < this->x_center && this->at(false))
	{
		return this->at(false)->search_point(point, result);
	}
	else if (point > this->x_center && this->at(true))
	{
		return this->at(true)->search_point(point, result);
	}
	return result;
}

template <class Key, class T>
IntervalTreeNode<Key, T>* IntervalTreeNode<Key, T>::prune()
{
	if (!this->at(false) || !this->at(true))
	{
		bool direction = !this->at(false);
		IntervalTreeNode<Key, T>* result = this->at(direction);
		return result;
	}
	else
	{
		auto pair = this->at(false)->pop_greatest_child();
		IntervalTreeNode<Key, T>* heir = pair.first;
		this->at(false) = pair.second;

		heir->at(false) = this->at(false);
		heir->at(true) = this->at(true);

		heir->refresh_balance();
		heir = heir->rotate();
		return heir;
	}
}

template <class Key, class T>
std::pair<IntervalTreeNode<Key, T>*, IntervalTreeNode<Key, T>*> IntervalTreeNode<Key, T>::pop_greatest_child()
{
	if (!this->right_node)
	{
		auto ivs = this->s_center.toList();
		std::sort(ivs.begin(), ivs.end(), sortByEndBegin<Key, T>);
		auto max_iv = ivs.takeLast();
		Key new_x_center = this->x_center;
		while (!ivs.isEmpty())
		{
			auto next_max_iv = ivs.takeLast();
			if (next_max_iv.end == max_iv.end)
			{
				continue;
			}
			new_x_center = std::max(new_x_center, next_max_iv.end);
		}

		auto get_new_s_center = [&]()->QList<Interval<Key, T>>
		{
			QList<Interval<Key, T>> result;
			for (const auto& iv : this->s_center)
			{
				if (iv.contains_point(new_x_center))
				{
					result.append(iv);
				}
			}
			return result;
		};

		IntervalTreeNode<Key, T>* child = new IntervalTreeNode<Key, T>(new_x_center, get_new_s_center());
		this->s_center -= child->s_center;
		if (!this->s_center.isEmpty())
		{
			return std::make_pair(child, this);
		}
		else
		{
			return std::make_pair(child, this->at(false));
		}
	}
	else
	{
		auto pair = this->at(true)->pop_greatest_child();
		IntervalTreeNode<Key, T>* greatest_child = pair.first;
		this->at(true) = pair.second;

		QSet<Interval<Key, T>> s_center_copy = this->s_center;
		for (const auto& iv : s_center_copy)
		{
			if (iv.contains_point(greatest_child->x_center))
			{
				this->s_center.remove(iv);
				greatest_child->add(iv);
			}
		}

		if (!this->s_center.isEmpty())
		{
			this->refresh_balance();
			IntervalTreeNode<Key, T>* new_self = this->rotate();
			return std::make_pair(greatest_child, new_self);
		}
		else
		{
			IntervalTreeNode<Key, T>* new_self = this->prune();
			return std::make_pair(greatest_child, new_self);
		}
	}
}

template <class Key, class T>
bool IntervalTreeNode<Key, T>::contains_point(const Key& p)
{
	for (const auto& iv : this->s_center)
	{
		if (iv.contains_point(p))
		{
			return true;
		}
	}
	IntervalTreeNode<Key, T>* branch = this->at(p > this->x_center);
	return branch && branch->contains_point(p);
}

template <class Key, class T>
QSet<Interval<Key, T>> IntervalTreeNode<Key, T>::all_children()
{
	return this->all_children_helper(QSet<Interval<Key, T>>());
}

template <class Key, class T>
QSet<Interval<Key, T>> IntervalTreeNode<Key, T>::all_children_helper(QSet<Interval<Key, T>>& result)
{
	result += this->s_center;
	if (this->at(false))
	{
		this->at(false)->all_children_helper(result);
	}
	if (this->at(true))
	{
		this->at(true)->all_children_helper(result);
	}
	return result;
}

template <class Key, class T>
void IntervalTreeNode<Key, T>::verify(const QSet<Key>& parents)
{

}

/*
在类中好像没法调用operator[]，因此改用at函数实现
对应__getitem__、__setitem__函数
*/
template <class Key, class T>
IntervalTreeNode<Key, T>* &IntervalTreeNode<Key, T>::at(bool index)
{
	if (index)
	{
		return this->right_node;
	}
	else
	{
		return this->left_node;
	}
}

template <class Key, class T>
QString IntervalTreeNode<Key, T>::str() const
{
	return QString("Node<%0, depth=%1, balance=%2>").arg(this->x_center)
		.arg(this->depth).arg(this->balance);
}

template <class Key, class T>
int IntervalTreeNode<Key, T>::count_nodes() const
{
	int count = 1;
	if (this->left_node)
	{
		count += this->left_node->count_nodes();
	}
	if (this->right_node)
	{
		count += this->right_node->count_nodes();
	}
	return count;
}

template <class Key, class T>
double IntervalTreeNode<Key, T>::depth_score(int n, int m) const
{
	int dopt = 1 + qFloor(std::log2(m));
	double f = 1.0 / static_cast<double>(1 + n - dopt);
	return f * this->depth_score_helper(1, dopt);
}

template <class Key, class T>
int IntervalTreeNode<Key, T>::depth_score_helper(int d, int dopt) const
{
	int di = d - dopt;
	int count;
	if (di > 0)
	{
		count = di * this->s_center.size();
	}
	else
	{
		count = 0;
	}
	if (this->right_node)
	{
		count += this->right_node->depth_score_helper(d + 1, dopt);
	}
	if (this->left_node)
	{
		count += this->left_node->depth_score_helper(d + 1, dopt);
	}
	return count;
}

