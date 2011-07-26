/*******************************************************************\

Module: Pointer Logic

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#include <assert.h>

#include <i2string.h>
#include <arith_tools.h>
#include <pointer_offset_size.h>
#include <std_expr.h>
#include <prefix.h>

#include "pointer_logic.h"

/*******************************************************************\

Function: pointer_logict::is_dynamic_object

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

bool pointer_logict::is_dynamic_object(const exprt &expr) const
{
  if(expr.type().dynamic()) return true;
  
  if(expr.id()=="symbol")
    if(has_prefix(id2string(to_symbol_expr(expr).get_identifier()),
                  "symex_dynamic::"))
      return true;

  return false;
}

/*******************************************************************\

Function: pointer_logict::get_dynamic_objects

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void pointer_logict::get_dynamic_objects(std::vector<unsigned> &o) const
{
  o.clear();
  unsigned nr=0;
  
  for(pointer_logict::objectst::const_iterator
      it=objects.begin();
      it!=objects.end();
      it++, nr++)
    if(is_dynamic_object(*it))
      o.push_back(nr);
}

/*******************************************************************\

Function: pointer_logict::add_object

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

unsigned pointer_logict::add_object(const exprt &expr)
{
  // remove any index/member
  
  if(expr.id()=="index")
  {
    assert(expr.operands().size()==2);
    return add_object(expr.op0());
  }
  else if(expr.id()=="member")
  {
    assert(expr.operands().size()==1);
    return add_object(expr.op0());
  }
  
  return objects.number(expr);
}

/*******************************************************************\

Function: pointer_logict::pointer_expr

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

exprt pointer_logict::pointer_expr(
  unsigned object,
  const typet &type) const
{
  pointert pointer(object, 0);
  return pointer_expr(pointer, type);
}

/*******************************************************************\

Function: pointer_logict::pointer_expr

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

exprt pointer_logict::pointer_expr(
  const pointert &pointer,
  const typet &type) const
{
  if(pointer.object==null_object) // NULL?
  {
    constant_exprt result(type);
    result.set_value("NULL");
    return result;
  }
  else if(pointer.object==invalid_object) // INVALID?
  {
    constant_exprt result(type);
    result.set_value("INVALID");
    return result;
  }
  
  if(pointer.object>=objects.size() || pointer.object<0)
  {
    constant_exprt result(type);
    result.set_value("INVALID-"+i2string(pointer.object));
    return result;
  }

  const exprt &object_expr=objects[pointer.object];

  exprt deep_object=object_rec(pointer.offset, type, object_expr);
  
  exprt result;
  
  if(type.id()=="pointer")
    result=exprt("address_of", type);
  else if(type.id()=="reference")
    result=exprt("reference_to", type);
  else
    assert(0);

  result.copy_to_operands(deep_object);
  return result;
}

/*******************************************************************\

Function: pointer_logict::object_rec

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

exprt pointer_logict::object_rec(
  const mp_integer &offset,
  const typet &pointer_type,
  const exprt &src) const
{
  assert(offset>=0);

  if(src.type().id()=="array")
  {
    mp_integer size=pointer_offset_size(src.type().subtype());

    if(size==0) return src;
    
    mp_integer index=offset/size;
    mp_integer rest=offset%size;

    index_exprt tmp(src.type().subtype());
    tmp.index()=from_integer(index, typet("integer"));
    tmp.array()=src;
    
    return object_rec(rest, pointer_type, tmp);
  }
  else if(src.type().id()=="struct" ||
          src.type().id()=="union")
  {
    assert(offset>=0);
  
    if(offset==0)
    {
      // the struct itself
      return src;
    }

    const irept::subt &components=
      src.type().find("components").get_sub();
      
    mp_integer current_offset=1;

    assert(offset>=current_offset);

    forall_irep(it, components)
    {
      assert(offset>=current_offset);

      const typet &subtype=it->type();

      mp_integer sub_size=pointer_offset_size(subtype);
      
      if(sub_size==0)
        return src;
      
      mp_integer new_offset=current_offset+sub_size;

      if(new_offset>offset)
      {
        // found it
        member_exprt tmp(subtype);
        tmp.set_component_name(it->name());
        tmp.op0()=src;
        
        return object_rec(
          offset-current_offset, pointer_type, tmp);
      }
      
      assert(new_offset<=offset);
      current_offset=new_offset;
      assert(current_offset<=offset);
    }
  }
  
  return src;
}

/*******************************************************************\

Function: pointer_logict::pointer_logict

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

pointer_logict::pointer_logict()
{
  // add NULL
  null_object=objects.number(exprt("NULL"));

  // add INVALID
  invalid_object=objects.number(exprt("INVALID"));
}

/*******************************************************************\

Function: pointer_logict::~pointer_logict

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

pointer_logict::~pointer_logict()
{
}
