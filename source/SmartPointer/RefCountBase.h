#pragma once
#include <xmemory>

namespace gmp {

class _Ref_count_base
{	// common code for reference counting
private:
	virtual void _Destroy() = 0;
	virtual void _Delete_this() = 0;

protected:
	_Ref_count_base()
	{	// construct
		_Uses = 1;
		_Weaks = 1;
	}

public:
	virtual ~_Ref_count_base()
	{	// ensure that derived classes can be destroyed properly
	}

	bool _Incref_nz()
	{	// increment use count if not zero, return true if successful
		for (; ; )
		{	// loop until state is known
			unsigned long _Count =
				static_cast<volatile unsigned long&>(_Uses);

			if (_Count == 0)
				return (false);

			if (static_cast<unsigned long>(_InterlockedCompareExchange(
				reinterpret_cast<volatile long *>(&_Uses),
				_Count + 1, _Count)) == _Count)
				return (true);
		}
	}

	void _Incref()
	{	// increment use count
		_InterlockedIncrement(reinterpret_cast<volatile long *>(&_Uses));
	}

	void _Incwref()
	{	// increment weak reference count
		_InterlockedIncrement(reinterpret_cast<volatile long *>(&_Weaks));
	}

	void _Decref()
	{	// decrement use count
		if (_InterlockedDecrement(reinterpret_cast<volatile long *>(&_Uses)) == 0)
		{	// destroy managed resource, decrement weak reference count
			_Destroy();
			_Decwref();
		}
	}

	void _Decwref()
	{	// decrement weak reference count
		if (_InterlockedDecrement(reinterpret_cast<volatile long *>(&_Weaks)) == 0)
			_Delete_this();
	}

	long _Use_count() const
	{	// return use count
		return (_Uses);
	}

	bool _Expired() const
	{	// return true if _Uses == 0
		return (_Use_count() == 0);
	}

	virtual void *_Get_deleter(const type_info&) const
	{	// return address of deleter object
		return (0);
	}

private:
	unsigned long _Uses;
	unsigned long _Weaks;
};

}