// ESPP_CLASS
#ifndef _INTEGRATOR_LANGEVINTHERMOSTAT_HPP
#define _INTEGRATOR_LANGEVINTHERMOSTAT_HPP

#include "types.hpp"
#include "logging.hpp"
#include "Particle.hpp"
#include "SystemAccess.hpp"

#include "Extension.hpp"
#include "VelocityVerlet.hpp"


#include "boost/signals2.hpp"


namespace espresso {
  namespace integrator {

    /** Langevin thermostat */

    class LangevinThermostat : public Extension {

      public:

        LangevinThermostat(shared_ptr<System> system);
        virtual ~LangevinThermostat();

        void setGamma(real gamma);
        real getGamma();

        void setTemperature(real temperature);
        real getTemperature();

        void setAdress(bool _adress);
        bool getAdress();

        void initialize();

        /** update of forces to thermalize the system */
        void thermalize();
        void thermalizeAdr(); // same as above, for AdResS

        /** very nasty: if we recalculate force when leaving/reentering the integrator,
            a(t) and a((t-dt)+dt) are NOT equal in the vv algorithm. The random
            numbers are drawn twice, resulting in a different variance of the random force.
            This is corrected by additional heat when restarting the integrator here.
            Currently only works for the Langevin thermostat, although probably also others
            are affected.
        */
        void heatUp();

        /** Opposite to heatUp */
        void coolDown();

        /** Register this class so it can be used from Python. */
        static void registerPython();

      private:

        boost::signals2::connection _initialize, _heatUp, _coolDown,
                                       _thermalize, _thermalizeAdr;

        void frictionThermo(class Particle&);

        // this connects thermalizeAdr
        void enableAdress();
        bool adress;

        void connect();
        void disconnect();

        real gamma;        //!< friction coefficient
        real temperature;  //!< desired user temperature

        real pref1;  //!< prefactor, reduces complexity of thermalize
        real pref2;  //!< prefactor, reduces complexity of thermalize

        real pref2buffer; //!< temporary to save value between heatUp/coolDown

        shared_ptr< esutil::RNG > rng;  //!< random number generator used for friction term

    };
  }
}

#endif
