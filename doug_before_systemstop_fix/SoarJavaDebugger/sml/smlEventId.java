/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version: 1.3.22
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package sml;

public final class smlEventId {
  public final static smlEventId smlEVENT_INVALID_EVENT = new smlEventId("smlEVENT_INVALID_EVENT", 0);
  public final static smlEventId smlEVENT_BEFORE_SHUTDOWN = new smlEventId("smlEVENT_BEFORE_SHUTDOWN", 1);
  public final static smlEventId smlEVENT_AFTER_CONNECTION_LOST = new smlEventId("smlEVENT_AFTER_CONNECTION_LOST");
  public final static smlEventId smlEVENT_BEFORE_RESTART = new smlEventId("smlEVENT_BEFORE_RESTART");
  public final static smlEventId smlEVENT_AFTER_RESTART = new smlEventId("smlEVENT_AFTER_RESTART");
  public final static smlEventId smlEVENT_BEFORE_RHS_FUNCTION_ADDED = new smlEventId("smlEVENT_BEFORE_RHS_FUNCTION_ADDED");
  public final static smlEventId smlEVENT_AFTER_RHS_FUNCTION_ADDED = new smlEventId("smlEVENT_AFTER_RHS_FUNCTION_ADDED");
  public final static smlEventId smlEVENT_BEFORE_RHS_FUNCTION_REMOVED = new smlEventId("smlEVENT_BEFORE_RHS_FUNCTION_REMOVED");
  public final static smlEventId smlEVENT_AFTER_RHS_FUNCTION_REMOVED = new smlEventId("smlEVENT_AFTER_RHS_FUNCTION_REMOVED");
  public final static smlEventId smlEVENT_BEFORE_RHS_FUNCTION_EXECUTED = new smlEventId("smlEVENT_BEFORE_RHS_FUNCTION_EXECUTED");
  public final static smlEventId smlEVENT_AFTER_RHS_FUNCTION_EXECUTED = new smlEventId("smlEVENT_AFTER_RHS_FUNCTION_EXECUTED");
  public final static smlEventId smlEVENT_BEFORE_SMALLEST_STEP = new smlEventId("smlEVENT_BEFORE_SMALLEST_STEP");
  public final static smlEventId smlEVENT_AFTER_SMALLEST_STEP = new smlEventId("smlEVENT_AFTER_SMALLEST_STEP");
  public final static smlEventId smlEVENT_BEFORE_ELABORATION_CYCLE = new smlEventId("smlEVENT_BEFORE_ELABORATION_CYCLE");
  public final static smlEventId smlEVENT_AFTER_ELABORATION_CYCLE = new smlEventId("smlEVENT_AFTER_ELABORATION_CYCLE");
  public final static smlEventId smlEVENT_BEFORE_PHASE_EXECUTED = new smlEventId("smlEVENT_BEFORE_PHASE_EXECUTED");
  public final static smlEventId smlEVENT_AFTER_PHASE_EXECUTED = new smlEventId("smlEVENT_AFTER_PHASE_EXECUTED");
  public final static smlEventId smlEVENT_BEFORE_DECISION_CYCLE = new smlEventId("smlEVENT_BEFORE_DECISION_CYCLE");
  public final static smlEventId smlEVENT_AFTER_DECISION_CYCLE = new smlEventId("smlEVENT_AFTER_DECISION_CYCLE");
  public final static smlEventId smlEVENT_AFTER_INTERRUPT = new smlEventId("smlEVENT_AFTER_INTERRUPT");
  public final static smlEventId smlEVENT_BEFORE_RUNNING = new smlEventId("smlEVENT_BEFORE_RUNNING");
  public final static smlEventId smlEVENT_AFTER_RUNNING = new smlEventId("smlEVENT_AFTER_RUNNING");
  public final static smlEventId smlEVENT_AFTER_PRODUCTION_ADDED = new smlEventId("smlEVENT_AFTER_PRODUCTION_ADDED");
  public final static smlEventId smlEVENT_BEFORE_PRODUCTION_REMOVED = new smlEventId("smlEVENT_BEFORE_PRODUCTION_REMOVED");
  public final static smlEventId smlEVENT_AFTER_PRODUCTION_FIRED = new smlEventId("smlEVENT_AFTER_PRODUCTION_FIRED");
  public final static smlEventId smlEVENT_BEFORE_PRODUCTION_RETRACTED = new smlEventId("smlEVENT_BEFORE_PRODUCTION_RETRACTED");
  public final static smlEventId smlEVENT_AFTER_AGENT_CREATED = new smlEventId("smlEVENT_AFTER_AGENT_CREATED");
  public final static smlEventId smlEVENT_BEFORE_AGENT_DESTROYED = new smlEventId("smlEVENT_BEFORE_AGENT_DESTROYED");
  public final static smlEventId smlEVENT_BEFORE_AGENT_REINITIALIZED = new smlEventId("smlEVENT_BEFORE_AGENT_REINITIALIZED");
  public final static smlEventId smlEVENT_AFTER_AGENT_REINITIALIZED = new smlEventId("smlEVENT_AFTER_AGENT_REINITIALIZED");
  public final static smlEventId smlEVENT_OUTPUT_PHASE_CALLBACK = new smlEventId("smlEVENT_OUTPUT_PHASE_CALLBACK");
  public final static smlEventId smlEVENT_LOG_ERROR = new smlEventId("smlEVENT_LOG_ERROR");
  public final static smlEventId smlEVENT_LOG_WARNING = new smlEventId("smlEVENT_LOG_WARNING");
  public final static smlEventId smlEVENT_LOG_INFO = new smlEventId("smlEVENT_LOG_INFO");
  public final static smlEventId smlEVENT_LOG_DEBUG = new smlEventId("smlEVENT_LOG_DEBUG");
  public final static smlEventId smlEVENT_PRINT = new smlEventId("smlEVENT_PRINT");
  public final static smlEventId smlEVENT_LAST = new smlEventId("smlEVENT_LAST");

  public final int swigValue() {
    return swigValue;
  }

  public String toString() {
    return swigName;
  }

  public static smlEventId swigToEnum(int swigValue) {
    if (swigValue < swigValues.length && swigValues[swigValue].swigValue == swigValue)
      return swigValues[swigValue];
    for (int i = 0; i < swigValues.length; i++)
      if (swigValues[i].swigValue == swigValue)
        return swigValues[i];
    throw new IllegalArgumentException("No enum " + smlEventId.class + " with value " + swigValue);
  }

  private smlEventId(String swigName) {
    this.swigName = swigName;
    this.swigValue = swigNext++;
  }

  private smlEventId(String swigName, int swigValue) {
    this.swigName = swigName;
    this.swigValue = swigValue;
    swigNext = swigValue+1;
  }

  private static smlEventId[] swigValues = { smlEVENT_INVALID_EVENT, smlEVENT_BEFORE_SHUTDOWN, smlEVENT_AFTER_CONNECTION_LOST, smlEVENT_BEFORE_RESTART, smlEVENT_AFTER_RESTART, smlEVENT_BEFORE_RHS_FUNCTION_ADDED, smlEVENT_AFTER_RHS_FUNCTION_ADDED, smlEVENT_BEFORE_RHS_FUNCTION_REMOVED, smlEVENT_AFTER_RHS_FUNCTION_REMOVED, smlEVENT_BEFORE_RHS_FUNCTION_EXECUTED, smlEVENT_AFTER_RHS_FUNCTION_EXECUTED, smlEVENT_BEFORE_SMALLEST_STEP, smlEVENT_AFTER_SMALLEST_STEP, smlEVENT_BEFORE_ELABORATION_CYCLE, smlEVENT_AFTER_ELABORATION_CYCLE, smlEVENT_BEFORE_PHASE_EXECUTED, smlEVENT_AFTER_PHASE_EXECUTED, smlEVENT_BEFORE_DECISION_CYCLE, smlEVENT_AFTER_DECISION_CYCLE, smlEVENT_AFTER_INTERRUPT, smlEVENT_BEFORE_RUNNING, smlEVENT_AFTER_RUNNING, smlEVENT_AFTER_PRODUCTION_ADDED, smlEVENT_BEFORE_PRODUCTION_REMOVED, smlEVENT_AFTER_PRODUCTION_FIRED, smlEVENT_BEFORE_PRODUCTION_RETRACTED, smlEVENT_AFTER_AGENT_CREATED, smlEVENT_BEFORE_AGENT_DESTROYED, smlEVENT_BEFORE_AGENT_REINITIALIZED, smlEVENT_AFTER_AGENT_REINITIALIZED, smlEVENT_OUTPUT_PHASE_CALLBACK, smlEVENT_LOG_ERROR, smlEVENT_LOG_WARNING, smlEVENT_LOG_INFO, smlEVENT_LOG_DEBUG, smlEVENT_PRINT, smlEVENT_LAST };
  private static int swigNext = 0;
  private final int swigValue;
  private final String swigName;
}


