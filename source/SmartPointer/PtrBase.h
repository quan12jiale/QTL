#pragma once
#include "RefCountBase.h"

namespace gmp {

template<class _Ty>
class _Ptr_base
{	// base class for shared_ptr and weak_ptr
public:
	typedef _Ptr_base<_Ty> _Myt;
	typedef _Ty element_type;

	constexpr _Ptr_base()
		: _Ptr(0), _Rep(0)
	{	// construct
	}

	long use_count() const
	{	// return use count
		return (_Rep ? _Rep->_Use_count() : 0);
	}

	void _Swap(_Ptr_base& _Right)
	{	// swap pointers
		std::swap(_Rep, _Right._Rep);
		std::swap(_Ptr, _Right._Ptr);
	}

	_Ty *_Get() const
	{	// return pointer to resource
		return (_Ptr);
	}

	void _Decref()
	{	// decrement reference count
		if (_Rep != 0)
			_Rep->_Decref();
	}

protected: // std中这里是私有的
	_Ty *_Ptr;
	_Ref_count_base *_Rep;
	template<class _Ty0>
		friend class _Ptr_base;
};

}