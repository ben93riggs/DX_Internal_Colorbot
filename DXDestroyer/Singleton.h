#pragma once

template<typename T>
class Singleton
{
protected:
	Singleton() = default;
	~Singleton() = default;

public:
	static T& Get()
	{
		static T inst{};
		return inst;
	}

	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;

	Singleton(Singleton&&) = delete;
	Singleton& operator=(Singleton&&) = delete;
};