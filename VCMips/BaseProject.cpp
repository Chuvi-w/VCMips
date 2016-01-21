#include "stdafx.h"
#include "BaseProject.h"


CBaseProject::CBaseProject(CBaseProject *Child)
{
	Base = Child;
}


CBaseProject::~CBaseProject()
{
	//Base->SaveProject();
}

