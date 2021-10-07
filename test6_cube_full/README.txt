SimpleVertexShader & SimpleFragmentShader: VS/FS pipe                         [cube_full.exe]
UBOSimpleVS & SimpleFragmentShader: VS/FS pipe, with UBO                  [cube_full.exe -u]

VS & GS & SimpleFragmentShader: VS/GS/FS pipe                                     [cube_full.exe --gs]
UBOVS & UBOGS & SimpleFragmentShader: VS/GS/FS pipe, with UBO      [cube_full.exe --gs -u]

VS & Tcs & Tes & SimpleFragmentShader: VS/Tess/FS pipe                                                [cube_full.exe --tess]
UBOVS & UBOTcs & UBOTes & SimpleFragmentShader: VS/Tess/FS pipe, with UBO         [cube_full.exe --tess -u]

VS & Tcs & TesForGS & SimpleFragmentShader: VS/Tess/GS/FS pipe                                            [cube_full.exe --tess --gs]
UBOVS & UBOTcs & UBOTesForGS & SimpleFragmentShader: VS/Tess/GS/FS pipe, with UBO     [cube_full.exe --tess --gs -u]
