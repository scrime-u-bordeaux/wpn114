/*
 * =====================================================================================
 *
 *       Filename:  controller_base.hpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  18.10.2017 21:39:31
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

namespace wpn114 {
namespace control {

class controller_base
{
public:
    virtual ~controller_base();
    virtual void send_back();
};

}
}