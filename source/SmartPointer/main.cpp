#include <memory>
#include "SharedPtr.h"


int main(int argc, char *argv[])
{

	gmp::shared_ptr<int> int_ptr(new int(2));

	return 0;
}