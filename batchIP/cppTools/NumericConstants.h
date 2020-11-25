#pragma once

#include "TemplateMetaprogramming.h"

namespace stdesque {
namespace numeric {

CONST_VAL_FUNC double pi()           { return 3.141592653589793; }

CONST_VAL_FUNC double twoPi()        { return 6.283185307179586; }

CONST_VAL_FUNC double oneThirdsPi()  { return 1.0471975511965976; }

CONST_VAL_FUNC double twoThirdsPi()  { return 2.0943951023931953; }

CONST_VAL_FUNC double fourThirdsPi() { return 4.1887902047863905; }

CONST_VAL_FUNC double oneThirds() { return 0.33333333333333; }

CONST_VAL_FUNC double twoThirds() { return 0.66666666666667; }

} // namespace numeric
} // stdesque

