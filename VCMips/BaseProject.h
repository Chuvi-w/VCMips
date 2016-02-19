#ifndef BaseProject_h__
#define BaseProject_h__


#define NotImplemented(...)\
{\
   _ftprintf(stderr, _T("Error: Function \"%s\" not implemented.\n"),__FUNCTION__);\
   return __VA_ARGS__;\
}


typedef void (*pfnForEachFileFunc)();

class CBaseProject
{
public:
	// make copy constructor and assignment operator inaccessible
	CBaseProject(CBaseProject *Child);
	~CBaseProject();
	virtual BOOL OpenProject(const TCHAR *ProjectName)=0;
	virtual BOOL SaveProject()=0;
   virtual BOOL CloseProject()=0;

private:
	// make copy constructor and assignment operator inaccessible
	CBaseProject(const CBaseProject &);
	CBaseProject &operator=(const CBaseProject &);

private:
	CBaseProject *Base;

};

#endif // BaseProject_h__
