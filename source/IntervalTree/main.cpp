#include "intervaltree.h"

int main(int argc, char *argv[])
{
	auto tree = new IntervalTree<int, bool>();
	for (int i = 0; i < 100; i++)
	{
		int begin = std::rand() % 100;
		tree->addi(begin, begin + 10, true);

		if (i == 40)
		{
			tree->addi(begin, begin + 11, true);
			tree->discardi(begin, begin + 11, true);
		}
	}

	QSet<Interval<int, bool>> tmpset;
	tmpset.insert(Interval<int, bool>(10, 15, true));
	tree->update(tmpset);

	int count = tree->top_node->count_nodes();

	delete tree;
	return 0;
}