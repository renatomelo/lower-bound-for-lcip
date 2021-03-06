#ifndef EASYSCIP_H
#define EASYSCIP_H
// EasySCIP 0.1
// A C++ interface to SCIP that is easy to use.
// based on the implementation of Ricardo Bittencourt
#include <vector>
#include <scip/scip.h>
#include "objscip/objscip.h"
#include "scip/cons_linear.h"
#include <scip/scipdefplugins.h>
#include <iostream>
#include <string>

namespace easyscip {
    class ScipCons;

    class ScipVar {
        public:
            ScipVar() : var(NULL) {
            }
            SCIP_VAR *var;
        friend ScipCons;
    };

    class ScipBinVar: public ScipVar {
        public:
            ScipBinVar(SCIP *scip, string name, double objective) {
                SCIPcreateVar(scip, &var, name.c_str(), 0.0, 1.0, objective, SCIP_VARTYPE_BINARY,
                        TRUE, FALSE, NULL, NULL, NULL, NULL, NULL);
                SCIPaddVar(scip, var);
            }
    };

    class ScipContVar: public ScipVar {
        public:
            ScipContVar(SCIP *scip, string name, double lower_bound, double upper_bound,
            double objective) {
                SCIPcreateVar(scip, &var, name.c_str(), lower_bound, upper_bound,
                    objective, SCIP_VARTYPE_CONTINUOUS, TRUE, FALSE, NULL, NULL, NULL, NULL, NULL);
                SCIPaddVar(scip, var);
            }
    };

    //class ScipPriceBinVar: public ScipVar {
    class ScipPriceVar: public ScipVar {
        public:
            ScipPriceVar(SCIP *scip, double objective) {
                SCIPcreateVar(scip, &var, NULL, 0.0, SCIPinfinity(scip), objective, SCIP_VARTYPE_CONTINUOUS,
                        TRUE, TRUE, NULL, NULL, NULL, NULL, NULL);
                SCIPaddPricedVar(scip, var, 1.0);
            }
    };

    class ScipIntVar : public ScipVar {
        public:
            ScipIntVar(SCIP *scip, string name, double lower_bound, double upper_bound,
            double objective) {
                SCIPcreateVar(scip, &var, name.c_str(), lower_bound, upper_bound,
                    objective, SCIP_VARTYPE_INTEGER, TRUE, FALSE, NULL, NULL, NULL, NULL, NULL);
                SCIPaddVar(scip, var);
            }
    };

    class ScipCons {
        public:
            SCIP_CONS *cons;
            void addVar(SCIP_VAR *var, double val) {
                SCIPaddCoefLinear(scip, cons, var, val);
            }
            void commit() {
                SCIPaddCons(scip, cons);
                //SCIPreleaseCons(scip, &cons);
            }
            ScipCons(SCIP *scip_, double lb, double ub, const char* name) : scip(scip_) {
                SCIPcreateConsLinear(scip, &cons, name, 0, NULL, NULL, lb, ub, TRUE, FALSE, TRUE, TRUE, TRUE,
                    FALSE, FALSE, FALSE, FALSE, FALSE);
                //SCIP_CALL(SCIPcreateConsLinear(scip, &cons, ("x(\\delta(" + to_string(l.vname[v]) + ")) == 2").c_str(), 0, NULL, NULL, 2.0, 2.0,
                //    TRUE, FALSE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE));
            }
            ScipCons() : cons(NULL) {
            }

        protected:
            SCIP *scip;
    };

    class ScipConsPrice : public ScipCons {
        public:
            ScipConsPrice(SCIP *scip_, double lb, double ub, const char *name){
                scip = scip_;
                SCIPcreateConsLinear(scip, &cons, name, 0, NULL, NULL, lb, ub, TRUE, FALSE, TRUE, TRUE, TRUE,
                    FALSE, TRUE, FALSE, FALSE, FALSE);
                //SCIP_CALL(SCIPcreateConsLinear(scip, &cons, ("x(\\delta(" + to_string(l.vname[v]) + ")) == 2").c_str(), 0, NULL, NULL, 2.0, 2.0,
                //    TRUE, FALSE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE));
            }
    };
}
#endif // EASYSCIP_H
