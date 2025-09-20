#pragma once

#include <string>

namespace Cerberus::DefaultAssets {
    const std::string CubeObj = R"(
# Default Cube with all vertex attributes (v/vt/vn)
# Provided by Cerberus Engine

v 1.0 1.0 -1.0
v 1.0 -1.0 -1.0
v 1.0 1.0 1.0
v 1.0 -1.0 1.0
v -1.0 1.0 -1.0
v -1.0 -1.0 -1.0
v -1.0 1.0 1.0
v -1.0 -1.0 1.0

vt 0.0 0.0
vt 1.0 0.0
vt 1.0 1.0
vt 0.0 1.0
vt 0.0 0.0
vt 1.0 0.0
vt 1.0 1.0
vt 0.0 1.0
vt 0.0 0.0
vt 1.0 0.0
vt 1.0 1.0
vt 0.0 1.0
vt 0.0 0.0
vt 1.0 0.0

vn 0.0 1.0 0.0
vn 0.0 0.0 1.0
vn -1.0 0.0 0.0
vn 0.0 -1.0 0.0
vn 1.0 0.0 0.0
vn 0.0 0.0 -1.0

f 5/1/1 3/2/1 1/3/1
f 3/2/2 8/3/2 4/4/2
f 7/5/3 6/6/3 8/7/3
f 2/8/4 8/9/4 6/10/4
f 1/11/5 4/12/5 2/13/5
f 5/1/1 7/14/1 3/2/1
f 3/2/2 7/5/2 8/3/2
f 7/5/3 5/1/3 6/6/3
f 2/8/4 4/12/4 8/9/4
f 1/11/5 3/2/5 4/12/5
)";

}