env = Environment()

import platform
import os

cpus = 0
if platform.system() == 'Windows':
    env['windows'] = True
    cpus = os.environ['NUMBER_OF_PROCESSORS']
else:
    env['windows'] = False
    cpus = os.sysconf('SC_NPROCESSORS_ONLN')

# max jobs?
if cpus <= 0:
    cpus = 1
else:
    cpus = int(float(cpus) * 1.5)

parallel = int(ARGUMENTS.get('parallel', -1))

if parallel == -1:
    SetOption('num_jobs', cpus)
elif parallel == 0:
    SetOption('num_jobs', 1)
else:
    SetOption('num_jobs', parallel)

# what are we building?
debug = ARGUMENTS.get('debug', 0)
release = ARGUMENTS.get('release', 0)

# default to building just debug version
if debug == 0 and release == 0:
    debug = 1

def buildProjects(env):
    xUnit = SConscript('xUnit++/sconscript', exports = 'env')
    env['xUnit'] = xUnit

    console = SConscript('xUnit++.console/sconscript', exports = 'env')
    Depends(console, xUnit)

    if ARGUMENTS.get('test', 1) == 1:
        bareTests = SConscript('Tests/BareTests/sconscript', exports = 'env')
        unitTests = SConscript('Tests/UnitTests/sconscript', exports = 'env')

        Depends(bareTests, [xUnit, console])
        Depends(unitTests, [xUnit, console])

        AddPostAction(bareTests, Action(str(console[0]) + " " + str(bareTests[0])))
        AddPostAction(unitTests, Action(str(console[0]) + " " + str(unitTests[0])))

if debug != 0:
    env['debug'] = True

    SConscript('.build/output.sconscript', exports = 'env')
    SConscript('.build/build.sconscript', exports = 'env')

    dbg = SConscript('.build/debug.sconscript', exports = 'env')
    buildProjects(dbg)

if release != 0:
    env['debug'] = False

    SConscript('.build/output.sconscript', exports = 'env')
    SConscript('.build/build.sconscript', exports = 'env')

    rel = SConscript('.build/release.sconscript', exports = 'env')
    buildProjects(rel)

