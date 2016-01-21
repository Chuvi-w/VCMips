// stdafx.h: включаемый файл дл€ стандартных системных включаемых файлов
// или включаемых файлов дл€ конкретного проекта, которые часто используютс€, но
// не часто измен€ютс€
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

// TODO: ”становите здесь ссылки на дополнительные заголовки, требующиес€ дл€ программы



#define DISPATCH_CAST(what) reinterpret_cast<IDispatch**>(&what)