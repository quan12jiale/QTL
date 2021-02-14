#include "intervaltree.h"

int main(int argc, char *argv[])
{
	QList<Interval<double, DummyIntervalData>> intervals;
	intervals.append(Interval<double, DummyIntervalData>(-1.1, 1.1, DummyIntervalData()));
	intervals.append(Interval<double, DummyIntervalData>(-0.5, 1.5, DummyIntervalData()));
	intervals.append(Interval<double, DummyIntervalData>(0.5, 1.7, DummyIntervalData()));
	auto tree = new IntervalTree<double, DummyIntervalData>(intervals);



	delete tree;
	return 0;
}