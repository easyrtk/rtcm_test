RTCM decoder and encoder took from rtklib
I am trying to modify it for self-contained codeset. Also will modify the data structure later.
It should be noted that
1) need define _CRT_SECURE_NO_WARNINGS in the PreprocessorDefinitions
2) need to add Winmm.lib in the input Link\AdditionalDependencies
