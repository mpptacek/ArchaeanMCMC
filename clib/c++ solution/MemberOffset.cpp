#include "stdafx.h"
#include "MemberOffset.h"

thread_local TempStrLoc::BUFFER_TYPE buff[TempStrLoc::BUFFER_SZ / sizeof(TempStrLoc::BUFFER_TYPE)];

void * TempStrLoc::Ptr() {
	return &buff;
};
