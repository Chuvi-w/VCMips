// stdafx.h: ���������� ���� ��� ����������� ��������� ���������� ������
// ��� ���������� ������ ��� ����������� �������, ������� ����� ������������, ��
// �� ����� ����������
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <atlbase.h>
//#include <atlsafe.h>
#include <atlconv.h>
#include <comdef.h>
#include <direct.h>
#include <exception>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <stddef.h>
#include <stdint.h>
// TODO: ���������� ����� ������ �� �������������� ���������, ����������� ��� ���������



#define DISPATCH_CAST(what) reinterpret_cast<IDispatch**>(&what)
#define ZeroVar(var) ZeroMemory(var,sizeof(var))