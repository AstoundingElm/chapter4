#pragma once

bool createCubeTextureImage(VulkanRenderDevice& vkDev, const char* filename, VkImage& textureImage, VkDeviceMemory& textureImageMemory, uint32_t* width = nullptr, uint32_t* height = nullptr);

bool createDepthResources(VulkanRenderDevice& vkDev, uint32_t width, uint32_t height, VulkanImage& depth)
{
	VkFormat depthFormat = findDepthFormat(vkDev.physicalDevice);
	
	if (!createImage(vkDev.device, vkDev.physicalDevice, width, height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depth.image, depth.imageMemory))
		return false;
	
	if (!createImageView(vkDev.device, depth.image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, &depth.imageView))
		return false;
	
	transitionImageLayout(vkDev, depth.image, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	
	return true;
}

class ModelRenderer: public RendererBase
{
	public:
	ModelRenderer(VulkanRenderDevice& vkDev, const char* modelFile, const char* textureFile, uint32_t uniformDataSize);
	ModelRenderer(VulkanRenderDevice& vkDev, bool useDepth, VkBuffer storageBuffer, VkDeviceMemory storageBufferMemory, uint32_t vertexBufferSize, uint32_t indexBufferSize, VulkanImage texture, VkSampler textureSampler, const std::vector<const char*>& shaderFiles, uint32_t uniformDataSize, bool useGeneralTextureLayout = true, VulkanImage externalDepth = { .image = VK_NULL_HANDLE }, bool deleteMeshData = true);
	virtual ~ModelRenderer();
	
	virtual void fillCommandBuffer(VkCommandBuffer commandBuffer, size_t currentImage) override;
	
	void updateUniformBuffer(VulkanRenderDevice& vkDev, uint32_t currentImage, const void* data, const size_t dataSize);
	
	// HACK to allow sharing textures between multiple ModelRenderers
	void freeTextureSampler() { textureSampler_ = VK_NULL_HANDLE; }
	
	private:
	bool useGeneralTextureLayout_ = false;
	bool isExternalDepth_ = false;
	bool deleteMeshData_ = true;
	
	size_t vertexBufferSize_;
	size_t indexBufferSize_;
	
	// 6. Storage Buffer with index and vertex data
	VkBuffer storageBuffer_;
	VkDeviceMemory storageBufferMemory_;
	
	VkSampler textureSampler_;
	VulkanImage texture_;
	
	bool createDescriptorSet(VulkanRenderDevice& vkDev, uint32_t uniformDataSize);
};

static constexpr VkClearColorValue clearValueColor = { 1.0f, 1.0f, 1.0f, 1.0f };

bool ModelRenderer::createDescriptorSet(VulkanRenderDevice& vkDev, uint32_t uniformDataSize)
{
	const std::array<VkDescriptorSetLayoutBinding, 4> bindings = {
		descriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT),
		descriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT),
		descriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT),
		descriptorSetLayoutBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
	};
	
	const VkDescriptorSetLayoutCreateInfo layoutInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.bindingCount = static_cast<uint32_t>(bindings.size()),
		.pBindings = bindings.data()
	};
	
	VK_CHECK(vkCreateDescriptorSetLayout(vkDev.device, &layoutInfo, nullptr, &descriptorSetLayout_));
	
	std::vector<VkDescriptorSetLayout> layouts(vkDev.swapchainImages.size(), descriptorSetLayout_);
	
	const VkDescriptorSetAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = nullptr,
		.descriptorPool = descriptorPool_,
		.descriptorSetCount = static_cast<uint32_t>(vkDev.swapchainImages.size()),
		.pSetLayouts = layouts.data()
	};
	
	descriptorSets_.resize(vkDev.swapchainImages.size());
	
	VK_CHECK(vkAllocateDescriptorSets(vkDev.device, &allocInfo, descriptorSets_.data()));
	
	for (size_t i = 0; i < vkDev.swapchainImages.size(); i++)
	{
		VkDescriptorSet ds = descriptorSets_[i];
		
		const VkDescriptorBufferInfo bufferInfo  = { uniformBuffers_[i], 0, uniformDataSize };
		const VkDescriptorBufferInfo bufferInfo2 = { storageBuffer_, 0, vertexBufferSize_ };
		const VkDescriptorBufferInfo bufferInfo3 = { storageBuffer_, vertexBufferSize_, indexBufferSize_ };
		const VkDescriptorImageInfo  imageInfo   = { textureSampler_, texture_.imageView, useGeneralTextureLayout_ ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
		
		const std::array<VkWriteDescriptorSet, 4> descriptorWrites = {
			bufferWriteDescriptorSet(ds, &bufferInfo,  0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
			bufferWriteDescriptorSet(ds, &bufferInfo2, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER),
			bufferWriteDescriptorSet(ds, &bufferInfo3, 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER),
			imageWriteDescriptorSet( ds, &imageInfo,   3)
		};
		
		vkUpdateDescriptorSets(vkDev.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
	
	return true;
}

void ModelRenderer::fillCommandBuffer(VkCommandBuffer commandBuffer, size_t currentImage)
{
	
	
	beginRenderPass(commandBuffer, currentImage);
	
	vkCmdDraw(commandBuffer, static_cast<uint32_t>(indexBufferSize_ / (sizeof(unsigned int))), 1, 0, 0);
	vkCmdEndRenderPass(commandBuffer);
}

void ModelRenderer::updateUniformBuffer(VulkanRenderDevice& vkDev, uint32_t currentImage, const void* data, const size_t dataSize)
{
	uploadBufferData(vkDev, uniformBuffersMemory_[currentImage], 0, data, dataSize);
}

ModelRenderer::ModelRenderer(VulkanRenderDevice& vkDev, const char* modelFile, const char* textureFile, uint32_t uniformDataSize)
: RendererBase(vkDev, VulkanImage())
{
	// Resource loading part
	if (!createTexturedVertexBuffer(vkDev, modelFile, &storageBuffer_, &storageBufferMemory_, &vertexBufferSize_, &indexBufferSize_))
	{
		printf("ModelRenderer: createTexturedVertexBuffer() failed\n");
		exit(EXIT_FAILURE);
	}
	
	createTextureImage(vkDev, textureFile, texture_.image, texture_.imageMemory);
	createImageView(vkDev.device, texture_.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, &texture_.imageView);
	createTextureSampler(vkDev.device, &textureSampler_);
	
	if (!createDepthResources(vkDev, vkDev.framebufferWidth, vkDev.framebufferHeight, depthTexture_) ||
		!createColorAndDepthRenderPass(vkDev, true, &renderPass_, RenderPassCreateInfo()) ||
		!createUniformBuffers(vkDev, uniformDataSize) ||
		!createColorAndDepthFramebuffers(vkDev, renderPass_, depthTexture_.imageView, swapchainFramebuffers_) ||
		!createDescriptorPool(vkDev, 1, 2, 1, &descriptorPool_) ||
		!createDescriptorSet(vkDev, uniformDataSize) ||
		!createPipelineLayout(vkDev.device, descriptorSetLayout_, &pipelineLayout_) ||
		!createGraphicsPipeline(vkDev, renderPass_, pipelineLayout_, {"data/shaders/chapter03/VK02.vert", "data/shaders/chapter03/VK02.frag", "data/shaders/chapter03/VK02.geom" }, &graphicsPipeline_))
	{
		printf("ModelRenderer: failed to create pipeline\n");
		exit(EXIT_FAILURE);
	}
}

ModelRenderer::ModelRenderer(VulkanRenderDevice& vkDev, bool useDepth, VkBuffer storageBuffer, VkDeviceMemory storageBufferMemory, uint32_t vertexBufferSize, uint32_t indexBufferSize, VulkanImage texture, VkSampler textureSampler, const std::vector<const char*>& shaderFiles, uint32_t uniformDataSize, bool useGeneralTextureLayout, VulkanImage externalDepth, bool deleteMeshData)
: useGeneralTextureLayout_(useGeneralTextureLayout)
, vertexBufferSize_(vertexBufferSize)
, indexBufferSize_(indexBufferSize)
, storageBuffer_(storageBuffer)
, storageBufferMemory_(storageBufferMemory)
, texture_(texture)
, textureSampler_(textureSampler)
, deleteMeshData_(deleteMeshData)
, RendererBase(vkDev, VulkanImage())
{
	if (useDepth)
	{
		isExternalDepth_ = (externalDepth.image != VK_NULL_HANDLE);
		
		if (isExternalDepth_)
			depthTexture_ = externalDepth;
		else
			createDepthResources(vkDev, vkDev.framebufferWidth, vkDev.framebufferHeight, depthTexture_);
	}
	
	if (	!createColorAndDepthRenderPass(vkDev, useDepth, &renderPass_, RenderPassCreateInfo()) ||
		!createUniformBuffers(vkDev, uniformDataSize) ||
		!createColorAndDepthFramebuffers(vkDev, renderPass_, depthTexture_.imageView, swapchainFramebuffers_) ||
		!createDescriptorPool(vkDev, 1, 2, 1, &descriptorPool_) ||
		!createDescriptorSet(vkDev, uniformDataSize) ||
		!createPipelineLayout(vkDev.device, descriptorSetLayout_, &pipelineLayout_) ||
		!createGraphicsPipeline(vkDev, renderPass_, pipelineLayout_, shaderFiles, &graphicsPipeline_))
	{                                                                                               
		printf("ModelRenderer: failed to create pipeline\n");
		exit(EXIT_FAILURE);
	}
}

ModelRenderer::~ModelRenderer()
{
	if (deleteMeshData_)
	{
		vkDestroyBuffer(device_, storageBuffer_, nullptr);
		vkFreeMemory(device_, storageBufferMemory_, nullptr);
	}
	
	if (textureSampler_ != VK_NULL_HANDLE)
	{
		vkDestroySampler(device_, textureSampler_, nullptr);
		destroyVulkanImage(device_, texture_);
	}
	
	if (!isExternalDepth_)
		destroyVulkanImage(device_, depthTexture_);
}


class CubeRenderer: public RendererBase
{
	public:
	CubeRenderer(VulkanRenderDevice& vkDev, VulkanImage inDepthTexture, const char* textureFile);
	virtual ~CubeRenderer();
	
	virtual void fillCommandBuffer(VkCommandBuffer commandBuffer, size_t currentImage) override;
	
	void updateUniformBuffer(VulkanRenderDevice& vkDev, uint32_t currentImage, const mat4& m);
	
	private:
	VkSampler textureSampler;
	VulkanImage texture;
	
	bool createDescriptorSet(VulkanRenderDevice& vkDev);
};

bool CubeRenderer::createDescriptorSet(VulkanRenderDevice& vkDev)
{
	const std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
		descriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT),
		descriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
	};
	
	const VkDescriptorSetLayoutCreateInfo layoutInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.bindingCount = static_cast<uint32_t>(bindings.size()),
		.pBindings = bindings.data()
	};
	
	VK_CHECK(vkCreateDescriptorSetLayout(vkDev.device, &layoutInfo, nullptr, &descriptorSetLayout_));
	
	std::vector<VkDescriptorSetLayout> layouts(vkDev.swapchainImages.size(), descriptorSetLayout_);
	
	const VkDescriptorSetAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = nullptr,
		.descriptorPool = descriptorPool_,
		.descriptorSetCount = static_cast<uint32_t>(vkDev.swapchainImages.size()),
		.pSetLayouts = layouts.data()
	};
	
	descriptorSets_.resize(vkDev.swapchainImages.size());
	
	VK_CHECK(vkAllocateDescriptorSets(vkDev.device, &allocInfo, descriptorSets_.data()));
	
	for (size_t i = 0; i < vkDev.swapchainImages.size(); i++)
	{
		VkDescriptorSet ds = descriptorSets_[i];
		
		const VkDescriptorBufferInfo bufferInfo  = { uniformBuffers_[i], 0, sizeof(mat4) };
		const VkDescriptorImageInfo  imageInfo   = { textureSampler, texture.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
		
		const std::array<VkWriteDescriptorSet, 2> descriptorWrites = {
			bufferWriteDescriptorSet(ds, &bufferInfo,  0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
			imageWriteDescriptorSet( ds, &imageInfo,   1)
		};
		
		vkUpdateDescriptorSets(vkDev.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
	
	return true;
}

bool createCubeTextureImage(VulkanRenderDevice& vkDev, const char* filename, VkImage& textureImage, VkDeviceMemory& textureImageMemory, uint32_t* width, uint32_t* height)
{
	int w, h, comp;
	const float* img = stbi_loadf(filename, &w, &h, &comp, 3);
	std::vector<float> img32(w * h * 4);
	
	float24to32(w, h, img, img32.data());
	
	if (!img) {
		printf("Failed to load [%s] texture\n", filename); fflush(stdout);
		return false;
	}
	
	stbi_image_free((void*)img);
	
	Bitmap in(w, h, 4, eBitmapFormat_Float, img32.data());
	Bitmap out = convertEquirectangularMapToVerticalCross(in);
	
	Bitmap cube = convertVerticalCrossToCubeMapFaces(out);
	
	if (width && height)
	{
		*width = w;
		*height = h;
	}
	
	return createTextureImageFromData(vkDev, textureImage, textureImageMemory,
									  cube.data_.data(), cube.w_, cube.h_,
									  VK_FORMAT_R32G32B32A32_SFLOAT,
									  6, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
}


void CubeRenderer::fillCommandBuffer(VkCommandBuffer commandBuffer, size_t currentImage)
{
	
	
	beginRenderPass(commandBuffer, currentImage);
	
	vkCmdDraw(commandBuffer, 36, 1, 0, 0);
	
	vkCmdEndRenderPass(commandBuffer);
}

void CubeRenderer::updateUniformBuffer(VulkanRenderDevice& vkDev, uint32_t currentImage, const mat4& m)
{
	uploadBufferData(vkDev, uniformBuffersMemory_[currentImage], 0, glm::value_ptr(m), sizeof(mat4));
}

CubeRenderer::CubeRenderer(VulkanRenderDevice& vkDev, VulkanImage inDepthTexture, const char* textureFile)
: RendererBase(vkDev, inDepthTexture)
{
	// Resource loading
	createCubeTextureImage(vkDev, textureFile, texture.image, texture.imageMemory);
	
	createImageView(vkDev.device, texture.image, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, &texture.imageView, VK_IMAGE_VIEW_TYPE_CUBE, 6);
	createTextureSampler(vkDev.device, &textureSampler);
	
	// Pipeline initialization
	if (!createColorAndDepthRenderPass(vkDev, true, &renderPass_, RenderPassCreateInfo()) ||
		!createUniformBuffers(vkDev, sizeof(mat4)) ||
		!createColorAndDepthFramebuffers(vkDev, renderPass_, depthTexture_.imageView, swapchainFramebuffers_) ||
		!createDescriptorPool(vkDev, 1, 0, 1, &descriptorPool_) ||
		!createDescriptorSet(vkDev) ||
		!createPipelineLayout(vkDev.device, descriptorSetLayout_, &pipelineLayout_) ||
		!createGraphicsPipeline(vkDev, renderPass_, pipelineLayout_, { "data/shaders/chapter04/VKCube.vert", "data/shaders/chapter04/VKCube.frag" }, &graphicsPipeline_))
	{
		printf("CubeRenderer: failed to create pipeline\n");
		exit(EXIT_FAILURE);
	}
}

CubeRenderer::~CubeRenderer()
{
	vkDestroySampler(device_, textureSampler, nullptr);
	destroyVulkanImage(device_, texture);
}




class VulkanClear: public RendererBase
{
	public:
	VulkanClear(VulkanRenderDevice& vkDev, VulkanImage depthTexture);
	
	virtual void fillCommandBuffer(VkCommandBuffer commandBuffer, size_t currentImage) override;
	
	private:
	bool shouldClearDepth;
};



VulkanClear::VulkanClear(VulkanRenderDevice& vkDev, VulkanImage depthTexture)
: RendererBase(vkDev, depthTexture)
, shouldClearDepth(depthTexture.image != VK_NULL_HANDLE)
{
	if (!createColorAndDepthRenderPass(
									   vkDev, shouldClearDepth, &renderPass_, RenderPassCreateInfo{ .clearColor_ = true, .clearDepth_ = true, .flags_ = eRenderPassBit_First }))
	{
		printf("VulkanClear: failed to create render pass\n");
		exit(EXIT_FAILURE);
	}
	
	createColorAndDepthFramebuffers(vkDev, renderPass_, depthTexture.imageView, swapchainFramebuffers_);
}

void VulkanClear::fillCommandBuffer(VkCommandBuffer commandBuffer, size_t swapFramebuffer)
{
	
	
	const VkClearValue clearValues[2] =
	{
		VkClearValue { .color = { 1.0f, 1.0f, 1.0f, 1.0f } },
		VkClearValue { .depthStencil = { 1.0f, 0 } }
	};
	
	const VkRect2D screenRect = {
		.offset = { 0, 0 },
		.extent = {.width = framebufferWidth_, .height = framebufferHeight_ }
	};
	
	const VkRenderPassBeginInfo renderPassInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = renderPass_,
		.framebuffer = swapchainFramebuffers_[swapFramebuffer],
		.renderArea = screenRect,
		.clearValueCount = static_cast<uint32_t>(shouldClearDepth ? 2 : 1),
		.pClearValues = &clearValues[0]
	};
	
	vkCmdBeginRenderPass( commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
	vkCmdEndRenderPass( commandBuffer );
}
