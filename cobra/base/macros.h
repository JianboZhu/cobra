// Author: Jianbo Zhu (jianbo.zhujb@gmail.com)
//
// Define various macros which are used widely in this project.

#ifndef BASE_MACROS_H_
#define BASE_MACROS_H_

// A macro to forbid the compiler to generate copy constrcutor
// and assign function which usually are error-prone.
//
// This macro declares but not implements these error-prone functions,
// so the linker will complain if we use them unknowingly.
#define DISABLE_COPY_AND_ASSIGN(name) \
  name(const name&); \
  void operator=(const name&)

// Get the resource content defined by blade resource_library rule.
#define STATIC_RESOURCE(resource) \
  base::StringPiece(RESOURCE_##resource, sizeof(RESOURCE_##resource) / sizeof(char))

#endif  // BASE_MACROS_H_
