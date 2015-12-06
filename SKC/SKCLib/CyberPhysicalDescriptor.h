
#ifndef CYBERPHYSICALDESCRIPTOR_H_
#define CYBERPHYSICALDESCRIPTOR_H_

#include <string>

#include "skc.h"

namespace skc{

using namespace std;

class CyberPhysicalDescriptor{
public:
	utility::string_t type= utility::string_t("");
	utility::string_t name = utility::string_t("");
	utility::string_t id = utility::string_t("");
	utility::string_t parentName = utility::string_t("");
	utility::string_t parentId = utility::string_t("");
	bool isPattern=false;

    static CyberPhysicalDescriptor * _default_cpd;

};



};

#endif /* CYBERPHYSICALDESCRIPTOR_H_ */
