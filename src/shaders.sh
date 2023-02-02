echo "Compiling shaders..."

rm *.spv

glslc -fshader-stage=vert data/VK02_ImGui.vert -o VK02_ImGui.vert.spv

glslc -fshader-stage=frag data/VK02_ImGui.frag -o VK02_ImGui.frag.spv

#glslc -fshader-stage=geom ../data/shaders/chapter03/VK02.geom -o #geom.spv


sleep .5