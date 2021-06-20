#pragma once
#include "RefCountBase.h"

namespace gmp {

template<class _Ty>
class _Ref_count
	: public _Ref_count_base
{	// handle reference counting for object without deleter
public:
	_Ref_count(_Ty *_Px)
		: _Ref_count_base(), _Ptr(_Px)
	{	// construct
	}

private:
	virtual void _Destroy() _NOEXCEPT
	{	// destroy managed resource
		delete _Ptr;
	}

	virtual void _Delete_this() _NOEXCEPT
	{	// destroy self
		delete this;
	}

	_Ty * _Ptr;
};

}