#pragma once

template<typename FuncType>
__forceinline static FuncType CallVFunction(void* ppClass, int index)
{
	return (*reinterpret_cast<FuncType**>(ppClass))[index];
}
