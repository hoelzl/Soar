/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.24
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package sml;

public class XMLEventHandlerPlusData extends EventHandlerPlusData {
  private long swigCPtr;

  protected XMLEventHandlerPlusData(long cPtr, boolean cMemoryOwn) {
    super(smlJNI.SWIGXMLEventHandlerPlusDataUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  protected static long getCPtr(XMLEventHandlerPlusData obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected XMLEventHandlerPlusData() {
    this(0, false);
  }

  protected void finalize() {
    delete();
  }

  public void delete() {
    if(swigCPtr != 0 && swigCMemOwn) {
      swigCMemOwn = false;
      smlJNI.delete_XMLEventHandlerPlusData(swigCPtr);
    }
    swigCPtr = 0;
    super.delete();
  }

  public void setM_Handler(SWIGTYPE_p_f_enum_sml__smlXMLEventId_p_void_p_sml__Agent_p_sml__ClientXML__void m_Handler) {
    smlJNI.set_XMLEventHandlerPlusData_m_Handler(swigCPtr, SWIGTYPE_p_f_enum_sml__smlXMLEventId_p_void_p_sml__Agent_p_sml__ClientXML__void.getCPtr(m_Handler));
  }

  public SWIGTYPE_p_f_enum_sml__smlXMLEventId_p_void_p_sml__Agent_p_sml__ClientXML__void getM_Handler() {
    long cPtr = smlJNI.get_XMLEventHandlerPlusData_m_Handler(swigCPtr);
    return (cPtr == 0) ? null : new SWIGTYPE_p_f_enum_sml__smlXMLEventId_p_void_p_sml__Agent_p_sml__ClientXML__void(cPtr, false);
  }

  public XMLEventHandlerPlusData(int eventID, SWIGTYPE_p_f_enum_sml__smlXMLEventId_p_void_p_sml__Agent_p_sml__ClientXML__void handler, SWIGTYPE_p_void userData, int callbackID) {
    this(smlJNI.new_XMLEventHandlerPlusData(eventID, SWIGTYPE_p_f_enum_sml__smlXMLEventId_p_void_p_sml__Agent_p_sml__ClientXML__void.getCPtr(handler), SWIGTYPE_p_void.getCPtr(userData), callbackID), true);
  }

}
