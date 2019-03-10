#pragma once
#include "RockDatabase.h"

struct KellerDatabase : public GenericRockDatabase {
public:
	KellerDatabase();
};

struct StandardGeochemDatabase : public GenericRockDatabase {
public:
	StandardGeochemDatabase();
};