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

// TODO: ���������� ����� ������ �� �������������� ���������, ����������� ��� ���������



#define DISPATCH_CAST(what) reinterpret_cast<IDispatch**>(&what)