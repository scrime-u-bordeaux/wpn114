/*
 * =====================================================================================
 *
 *       Filename:  sndf.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  15.10.2017 17:46:12
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <string>

typedef short*  sndbuf16_t;
typedef int*    sndbuf32_t;

template<typename T> static T load_soundfile(std::string& path_to_soundfile);
