#pragma once
class RendererBase
{
	public:
	explicit RendererBase(const VulkanRenderDevice& vkDev, VulkanImage depthTexture)
		: device_(vkDev.device)
		, framebufferWidth_(vkDev.framebufferWidth)
		, framebufferHeight_(vkDev.framebufferHeight)
		, depthTexture_(depthTexture)
	{}
	virtual ~RendererBase();
	virtual void fillCommandBuffer(VkCommandBuffer commandBuffer, size_t currentImage) = 0;
	
	inline VulkanImage getDepthTexture() const { return depthTexture_; }
	
	protected:
	void beginRenderPass(VkCommandBuffer commandBuffer, size_t currentImage);
	bool createUniformBuffers(VulkanRenderDevice& vkDev, size_t uniformDataSize);
	
	VkDevice device_ = nullptr;
	
	uint32_t framebufferWidth_ = 0;
	uint32_t framebufferHeight_ = 0;
	
	// Depth buffer
	VulkanImage depthTexture_;
	
	// Descriptor set (layout + pool + sets) -> uses uniform buffers, textures, framebuffers
	VkDescriptorSetLayout descriptorSetLayout_ = nullptr;
	VkDescriptorPool descriptorPool_ = nullptr;
	std::vector<VkDescriptorSet> descriptorSets_;
	
	// Framebuffers (one for each command buffer)
	std::vector<VkFramebuffer> swapchainFramebuffers_;
	
	// 4. Pipeline & render pass (using DescriptorSets & pipeline state options)
	VkRenderPass renderPass_ = nullptr;
	VkPipelineLayout pipelineLayout_ = nullptr;
	VkPipeline graphicsPipeline_ = nullptr;
	
	// 5. Uniform buffer
	std::vector<VkBuffer> uniformBuffers_;
	std::vector<VkDeviceMemory> uniformBuffersMemory_;
};

RendererBase::~RendererBase()
{
	for (auto buf : uniformBuffers_)
		vkDestroyBuffer(device_, buf, nullptr);
	
	for (auto mem : uniformBuffersMemory_)
		vkFreeMemory(device_, mem, nullptr);
	
	vkDestroyDescriptorSetLayout(device_, descriptorSetLayout_, nullptr);
	vkDestroyDescriptorPool(device_, descriptorPool_, nullptr);
	
	for (auto framebuffer : swapchainFramebuffers_)
		vkDestroyFramebuffer(device_, framebuffer, nullptr);
	
	vkDestroyRenderPass(device_, renderPass_, nullptr);
	vkDestroyPipelineLayout(device_, pipelineLayout_, nullptr);
	vkDestroyPipeline(device_, graphicsPipeline_, nullptr);
}

void RendererBase::beginRenderPass(VkCommandBuffer commandBuffer, size_t currentImage)
{
	const VkRect2D screenRect = {
		.offset = { 0, 0 },
		.extent = {.width = framebufferWidth_, .height = framebufferHeight_ }
	};
	
	const VkRenderPassBeginInfo renderPassInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.pNext = nullptr,
		.renderPass = renderPass_,
		.framebuffer = swapchainFramebuffers_[currentImage],
		.renderArea = screenRect
	};
	
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline_);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, 1, &descriptorSets_[currentImage], 0, nullptr);
}

bool RendererBase::createUniformBuffers(VulkanRenderDevice& vkDev, size_t uniformDataSize)
{
	uniformBuffers_.resize(vkDev.swapchainImages.size());
	uniformBuffersMemory_.resize(vkDev.swapchainImages.size());
	for (size_t i = 0; i < vkDev.swapchainImages.size(); i++)
	{
		if (!createUniformBuffer(vkDev, uniformBuffers_[i], uniformBuffersMemory_[i], uniformDataSize))
		{
			printf("Cannot create uniform buffer\n");
			fflush(stdout);
			return false;
		}
	}
	return true;
}
