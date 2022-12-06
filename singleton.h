#pragma once
template <class T>
class SingleTon
{
public:
	inline static T& instance()
	{
		static T inst;
		return inst;
	}

protected:
	SingleTon(void) {}
	virtual ~SingleTon(void) {}
	SingleTon(const SingleTon<T>&) = delete;
	SingleTon<T>& operator =(const SingleTon<T>&) = delete;
};
