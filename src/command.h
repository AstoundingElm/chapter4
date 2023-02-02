#pragma once

VkCommandBuffer beginSingleTimeCommands(VulkanRenderDevice& vkDev)
{
	VkCommandBuffer commandBuffer;
	
	const VkCommandBufferAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = vkDev.commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};
	
	vkAllocateCommandBuffers(vkDev.device, &allocInfo, &commandBuffer);
	
	const VkCommandBufferBeginInfo beginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = nullptr
	};
	
	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	
	return commandBuffer;
}


void endSingleTimeCommands(VulkanRenderDevice& vkDev, VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);
	
	const VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = nullptr,
		.waitSemaphoreCount = 0,
		.pWaitSemaphores = nullptr,
		.pWaitDstStageMask = nullptr,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer,
		.signalSemaphoreCount = 0,
		.pSignalSemaphores = nullptr
	};
	
	vkQueueSubmit(vkDev.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(vkDev.graphicsQueue);
	
	vkFreeCommandBuffers(vkDev.device, vkDev.commandPool, 1, &commandBuffer);
}