#pragma once

struct shader_file_info{
    
    
	
    FILE * vert_file;
    FILE * frag_file;
	FILE* geom_file;
    u32 vert_size;
    u32 frag_size;
	u32 geom_size;
    char * vert_code;
    char * frag_code;
	char* geom_code;
    
    
};

static shader_file_info shader_data = {};

PINLINE bool read_shader_file(){
	shader_data.vert_file = NULL;
	shader_data.frag_file = NULL;
	shader_data.geom_file = NULL;
	shader_data.vert_file = fopen("/home/petermiller/Desktop/4coder/chapter4/src/VK02_ImGui.vert.spv", "rb+");
	if(!shader_data.vert_file){
		printf("fail");
	}
	shader_data.frag_file = fopen("/home/petermiller/Desktop/4coder/chapter4/src/VK02_ImGui.frag.spv", "rb+");
	/*
	shader_data.geom_file = fopen("/home/petermiller/Desktop/4coder/grcb/src/geom.spv", "rb+");*/
	
	if (shader_data.vert_file == NULL || shader_data.frag_file == NULL) {
		
		printf("could loadnt spv files");
		return false;
	};
	fseek(shader_data.vert_file, 0, SEEK_END);
	fseek(shader_data.frag_file, 0, SEEK_END);
	//fseek(shader_data.geom_file, 0, SEEK_END);
	
	shader_data.vert_size = ftell(shader_data.vert_file);
	shader_data.frag_size = ftell(shader_data.frag_file);
	//shader_data.geom_size = ftell(shader_data.geom_file);
	
	shader_data.vert_code = (char *)malloc(shader_data.vert_size * sizeof(char));
	shader_data.frag_code = (char *)malloc(shader_data.frag_size * sizeof(char));
	//shader_data.geom_code = (char *)malloc(shader_data.geom_size * sizeof(char));
	
	rewind(shader_data.vert_file);
	rewind(shader_data.frag_file);
	//rewind(shader_data.geom_file);
	fread(shader_data.vert_code, 1, shader_data.vert_size, shader_data.vert_file);
	fread(shader_data.frag_code, 1, shader_data.frag_size, shader_data.frag_file);
	//fread(shader_data.geom_code, 1, shader_data.geom_size, shader_data.geom_file);
	
	fclose(shader_data.vert_file);
	fclose(shader_data.frag_file);
	//fclose(shader_data.geom_file);
	return true;
};





PINLINE VkPipelineShaderStageCreateInfo create_pipeline_shader_stage_info(const void * pipeline_shader_stage_pnext, 
                                                                          VkPipelineShaderStageCreateFlags pipeline_shader_stage_flags, VkShaderStageFlagBits pipeline_shader_stage_flag_bits,
                                                                          VkShaderModule pipeline_shader_stage_module, const char * pipeline_shader_stage_name, 
                                                                          const VkSpecializationInfo * pipeline_shader_stage_specialization_info){
	VkPipelineShaderStageCreateInfo pipeline_shader_stage_info = {};
	pipeline_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipeline_shader_stage_info.pNext = pipeline_shader_stage_pnext;
	pipeline_shader_stage_info.flags = pipeline_shader_stage_flags;
	pipeline_shader_stage_info.stage = pipeline_shader_stage_flag_bits;
	pipeline_shader_stage_info.module = pipeline_shader_stage_module;
	pipeline_shader_stage_info.pName = pipeline_shader_stage_name;
	pipeline_shader_stage_info.pSpecializationInfo = pipeline_shader_stage_specialization_info;
	
	return pipeline_shader_stage_info;
	
}

PINLINE VkShaderModuleCreateInfo create_shader_module_info(const void * shader_module_pnext, VkShaderModuleCreateFlags shader_module_flags, 
                                                           size_t shader_code_size, const u32 * shader_module_pcode){
	
	VkShaderModuleCreateInfo shader_module_info = {};
	shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_module_info.pNext = shader_module_pnext;
	shader_module_info.flags = shader_module_flags;
	shader_module_info.codeSize = shader_code_size;
	shader_module_info.pCode = shader_module_pcode;
	
	return shader_module_info;
	
};

bool createPipelineLayoutWithConstants(VkDevice device, VkDescriptorSetLayout dsLayout, VkPipelineLayout* pipelineLayout, uint32_t vtxConstSize, uint32_t fragConstSize)
{
	
	
	const VkPushConstantRange ranges[] =
	{
		{
			VK_SHADER_STAGE_VERTEX_BIT,   // stageFlags
			0,                            // offset
			vtxConstSize                  // size
		},
		
		{
			VK_SHADER_STAGE_FRAGMENT_BIT, // stageFlags
			vtxConstSize,                 // offset
			fragConstSize                 // size
		}
	};
	
	uint32_t constSize = (vtxConstSize > 0) + (fragConstSize > 0);
	
	const VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.setLayoutCount = 1,
		.pSetLayouts = &dsLayout,
		.pushConstantRangeCount = constSize,
		.pPushConstantRanges = (constSize == 0) ? nullptr :
		(vtxConstSize > 0 ? ranges : &ranges[1])
	};
	
	return (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, pipelineLayout) == VK_SUCCESS);
}


bool createGraphicsPipeline(
							VulkanRenderDevice& vkDev,
							VkRenderPass renderPass, VkPipelineLayout pipelineLayout,
							const std::vector<const char*>& shaderFiles,
							VkPipeline* pipeline,
							VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST ,
							bool useDepth = true,
							bool useBlending = true,
							bool dynamicScissorState = false,
							int32_t customWidth  = -1,
							int32_t customHeight = -1,
							uint32_t numPatchControlPoints = 0);

bool createGraphicsPipeline(
							VulkanRenderDevice& vkDev,
							VkRenderPass renderPass, VkPipelineLayout pipelineLayout,
							const std::vector<const char*>& shaderFiles,
							VkPipeline* pipeline,
							VkPrimitiveTopology topology,
							bool useDepth,
							bool useBlending,
							bool dynamicScissorState,
							int32_t customWidth,
							int32_t customHeight,
							uint32_t numPatchControlPoints)
{
	
	bool shad = read_shader_file();
	if(!shad)
	{
		printf("Failed");
	}
	VkShaderModule vertMod = {};
	VkShaderModule fragMod = {};
	//VkShaderModule geomMod = {};
	
	
	VkShaderModuleCreateInfo vertInfo = create_shader_module_info(NULL, 0, shader_data.vert_size, (const u32*)shader_data.vert_code);
	
	
	vkCreateShaderModule(vkDev.device, &vertInfo, nullptr, &vertMod);
	
	const VkShaderModuleCreateInfo fragInfo =
	{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = shader_data.frag_size,
		.pCode = (uint32_t*)shader_data.frag_code,
	};
	
	vkCreateShaderModule(vkDev.device, &fragInfo, nullptr, &fragMod);
	
	
	/*const VkShaderModuleCreateInfo geomInfo =
	{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = shader_data.geom_size,
		.pCode = (uint32_t*)shader_data.geom_code,
	};
	
	vkCreateShaderModule(vkDev.device, &geomInfo, nullptr, &geomMod);*/
	
	VkPipelineShaderStageCreateInfo vertShaderInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module =  vertMod,
		.pName = "main",
		.pSpecializationInfo = nullptr
	};
	
	
	VkPipelineShaderStageCreateInfo fragShaderInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module =  fragMod,
		.pName = "main",
		.pSpecializationInfo = nullptr
	};
	
	/*
	VkPipelineShaderStageCreateInfo geomShaderInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.stage = VK_SHADER_STAGE_GEOMETRY_BIT,
		.module =  geomMod,
		.pName = "main",
		.pSpecializationInfo = nullptr
	};*/
	
	VkPipelineShaderStageCreateInfo shaderStages[3]  { vertShaderInfo, fragShaderInfo};
	
	
	const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
	};
	
	const VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		/* The only difference from createGraphicsPipeline() */
		.topology = topology,
		.primitiveRestartEnable = VK_FALSE
	};
	
	const VkViewport viewport = {
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<float>(customWidth > 0 ? customWidth : vkDev.framebufferWidth),
		.height = static_cast<float>(customHeight > 0 ? customHeight : vkDev.framebufferHeight),
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};
	
	const VkRect2D scissor = {
		.offset = { 0, 0 },
		.extent = { customWidth > 0 ? customWidth : vkDev.framebufferWidth, customHeight > 0 ? customHeight : vkDev.framebufferHeight }
	};
	
	const VkPipelineViewportStateCreateInfo viewportState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor
	};
	
	const VkPipelineRasterizationStateCreateInfo rasterizer = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_NONE,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.lineWidth = 1.0f
	};
	
	const VkPipelineMultisampleStateCreateInfo multisampling = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
		.minSampleShading = 1.0f
	};
	
	const VkPipelineColorBlendAttachmentState colorBlendAttachment = {
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = useBlending ? VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA : VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};
	
	const VkPipelineColorBlendStateCreateInfo colorBlending = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment,
		.blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f }
	};
	
	const VkPipelineDepthStencilStateCreateInfo depthStencil = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = static_cast<VkBool32>(useDepth ? VK_TRUE : VK_FALSE),
		.depthWriteEnable = static_cast<VkBool32>(useDepth ? VK_TRUE : VK_FALSE),
		.depthCompareOp = VK_COMPARE_OP_LESS,
		.depthBoundsTestEnable = VK_FALSE,
		.minDepthBounds = 0.0f,
		.maxDepthBounds = 1.0f
	};
	
	VkDynamicState dynamicStateElt = VK_DYNAMIC_STATE_SCISSOR;
	
	const VkPipelineDynamicStateCreateInfo dynamicState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.dynamicStateCount = 1,
		.pDynamicStates = &dynamicStateElt
	};
	
	const VkPipelineTessellationStateCreateInfo tessellationState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.patchControlPoints = numPatchControlPoints
	};
	
	const VkGraphicsPipelineCreateInfo pipelineInfo = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = ArraySize(shaderStages),
		.pStages = shaderStages,
		.pVertexInputState = &vertexInputInfo,
		.pInputAssemblyState = &inputAssembly,
		.pTessellationState = (topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST) ? &tessellationState : nullptr,
		.pViewportState = &viewportState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pDepthStencilState = useDepth ? &depthStencil : nullptr,
		.pColorBlendState = &colorBlending,
		.pDynamicState = dynamicScissorState ? &dynamicState : nullptr,
		.layout = pipelineLayout,
		.renderPass = renderPass,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1
	};
	
	VK_CHECK(vkCreateGraphicsPipelines(vkDev.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, pipeline));
	
	//for (auto m: shaderModules)
	//vkDestroyShaderModule(vkDev.device, m.shaderModule, nullptr);
	
	return true;
}