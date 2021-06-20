#pragma once
#include "RefCount.h"
#include "PtrBase.h"

namespace gmp {

template<class _Ty>
class shared_ptr
	: public _Ptr_base<_Ty>
{	// class for reference counted resource management
public:
	typedef shared_ptr<_Ty> _Myt;
	typedef _Ptr_base<_Ty> _Mybase;

	constexpr shared_ptr()
	{	// construct empty shared_ptr
	}

	template<class _Ux>
	explicit shared_ptr(_Ux *_Px)
	{	// construct shared_ptr object that owns _Px
		_Resetp(_Px);
	}

	void _Reset0(_Ty *_Other_ptr, _Ref_count_base *_Other_rep)
	{	// release resource and take new resource
		if (_Rep != 0)
			_Rep->_Decref();
		_Rep = _Other_rep;
		_Ptr = _Other_ptr;
	}

private:
	template<class _Ux>
	void _Resetp(_Ux *_Px)
	{	// release, take ownership of _Px
		try {	// allocate control block and reset
			_Resetp0(_Px, new _Ref_count<_Ux>(_Px));
		}
		catch (...) {	// allocation failed, delete resource
			delete _Px;
			throw;
		}
	}

public:
	template<class _Ux>
	void _Resetp0(_Ux *_Px, _Ref_count_base *_Rx)
	{	// release resource and take ownership of _Px
		this->_Reset0(_Px, _Rx);
		//_Enable_shared(_Px, _Rx);
	}

};

}