#include "intervaltree.h"

int main(int argc, char *argv[])
{
	auto tree = new IntervalTree<double, bool>();
	tree->addi(-1.1, 1.1, true);
	tree->addi(-0.5, 1.5, true);
	tree->addi(0.5, 1.7, true);

	tree->remove_overlap(1.1);
// 	for (int i = 0; i < 100; i++)
// 	{
// 		int begin = std::rand() % 100;
// 		tree->addi(begin, begin + 10, true);
// 	}

	auto tree2 = new IntervalTree<double, DummyIntervalData>();

	delete tree;
	return 0;
}