// ESPP_CLASS
#ifndef _ANALYSIS_ANALYSISBASE_HPP
#define _ANALYSIS_ANALYSISBASE_HPP
#include "types.hpp"
#include "python.hpp"
//#include "SystemAccess.hpp"
#include "ParticleAccess.hpp"
#include <limits>

namespace espresso {
  namespace analysis {

    /** All quantities to be measured derive from this abstract base class and the
    * corresponding template.
    * The following functions have to be implemented by the derived class:
    * virtual ResultType computeRaw()
    * virtual python::list compute()
    * virtual python::list getAverageValue()
    * virtual void resetAverage()
    * virtual void updateAverage(ResultType res) = 0;
    * */
      class AnalysisBase : public ParticleAccess{ //SystemAccess {
      public:
        AnalysisBase(shared_ptr< System > system) : ParticleAccess(system) {}
        virtual ~AnalysisBase() {}
      virtual void performMeasurement() = 0;
      virtual void reset() = 0;
      virtual python::list compute() = 0;
      virtual python::list getAverageValue() = 0;
      virtual int getNumberOfMeasurements() = 0;
      
      void perform_action(){
        performMeasurement();
      }
      
      static void registerPython();
      static LOG4ESPP_DECL_LOGGER(logger);
    };

    template < class ResultType >
    class AnalysisBaseTemplate : public AnalysisBase {
    public:
      AnalysisBaseTemplate(shared_ptr< System > system) : AnalysisBase(system) {
    	  //this->reset();
      };
      virtual ~AnalysisBaseTemplate() {};
      virtual void performMeasurement();
      virtual void reset();
      virtual int getNumberOfMeasurements();
      virtual python::list compute() = 0;
      virtual python::list getAverageValue() = 0;
      virtual void resetAverage() = 0;
      virtual ResultType computeRaw() = 0;
      virtual void updateAverage(ResultType res) = 0;

    protected:
      ResultType newAverage, lastAverage;
      ResultType newVariance, lastVariance;
      int nMeasurements;
    };

    /** This method will measure the observable and can be called from within the integrator
     * by using the ExtAnalyse extension. It can also be called from python.
     * It will also update the average value (typically also the variance) of the observable **/
    template < class ResultType >
    inline void
    AnalysisBaseTemplate< ResultType >::performMeasurement() {
    	ResultType res = computeRaw();
    	nMeasurements++;
    	updateAverage(res);
    }

    /** This method resets/initializes the measurement **/
    template < class ResultType >
    inline void
    AnalysisBaseTemplate< ResultType >::reset() {
    	nMeasurements = 0;
    	resetAverage();
    };

    /** return the number of measurements since the last reset **/
    template < class ResultType >
    inline int
    AnalysisBaseTemplate< ResultType >::getNumberOfMeasurements() {
      return nMeasurements;
    };
  }
}
#endif
