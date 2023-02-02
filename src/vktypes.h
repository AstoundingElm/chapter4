#pragma once


struct VulkanInstance final
{
	VkInstance instance;
	VkSurfaceKHR surface;
	VkDebugUtilsMessengerEXT messenger;
	VkDebugReportCallbackEXT reportCallback;
};

struct VulkanRenderDevice final
{
	uint32_t framebufferWidth;
	uint32_t framebufferHeight;
	
	VkDevice device;
	VkQueue graphicsQueue;
	VkPhysicalDevice physicalDevice;
	
	uint32_t graphicsFamily;
	
	VkSwapchainKHR swapchain;
	VkSemaphore semaphore;
	VkSemaphore renderSemaphore;
	
	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;
	
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	
	// For chapter5/6etc (compute shaders)
	
	// Were we initialized with compute capabilities
	bool useCompute = false;
	
	// [may coincide with graphicsFamily]
	uint32_t computeFamily;
	VkQueue computeQueue;
	
	// a list of all queues (for shared buffer allocation)
	std::vector<uint32_t> deviceQueueIndices;
	std::vector<VkQueue> deviceQueues;
	
	VkCommandBuffer computeCommandBuffer;
	VkCommandPool computeCommandPool;
};

// Features we need for our Vulkan context
struct VulkanContextFeatures
{
	bool supportScreenshots_ = false;
	
	bool geometryShader_    = true;
	bool tessellationShader_ = false;
	
	bool vertexPipelineStoresAndAtomics_ = false;
	bool fragmentStoresAndAtomics_ = false;
};

/* To avoid breaking chapter 1-6 samples, we introduce a class which differs from VulkanInstance in that it has a ctor & dtor */
struct VulkanContextCreator
{
	VulkanContextCreator() = default;
	
	VulkanContextCreator(VulkanInstance& vk, VulkanRenderDevice& dev, void* window, int screenWidth, int screenHeight, const VulkanContextFeatures& ctxFeatures = VulkanContextFeatures());
	~VulkanContextCreator();
	
	VulkanInstance& instance;
	VulkanRenderDevice& vkDev;
};

struct SwapchainSupportDetails final
{
	VkSurfaceCapabilitiesKHR capabilities = {};
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct ShaderModule final
{
	std::vector<unsigned int> SPIRV;
	VkShaderModule shaderModule = nullptr;
};

struct VulkanBuffer
{
	VkBuffer       buffer;
	VkDeviceSize   size;
	VkDeviceMemory memory;
	
	/* Permanent mapping to CPU address space (see VulkanResources::addBuffer) */
	void*          ptr;
};

struct VulkanImage final
{
	VkImage image = nullptr;
	VkDeviceMemory imageMemory = nullptr;
	VkImageView imageView = nullptr;
};

// Aggregate structure for passing around the texture data
struct VulkanTexture final
{
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	VkFormat format;
	
	VulkanImage image;
	VkSampler sampler;
	
	// Offscreen buffers require VK_IMAGE_LAYOUT_GENERAL && static textures have VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	VkImageLayout desiredLayout;
};

enum eRenderPassBit : uint8_t
{
	eRenderPassBit_First     = 0x01,
	eRenderPassBit_Last      = 0x02,
	eRenderPassBit_Offscreen = 0x04,
	eRenderPassBit_OffscreenInternal = 0x08,
};

struct RenderPassCreateInfo final
{
	bool clearColor_ = false;
	bool clearDepth_ = false;
	uint8_t flags_ = 0;
};