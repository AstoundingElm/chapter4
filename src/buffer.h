#pragma once

bool createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	const VkBufferCreateInfo bufferInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.size = size,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = nullptr
	};
	
	VK_CHECK(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer));
	
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
	
	const VkMemoryAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = nullptr,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties)
	};
	
	VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory));
	
	vkBindBufferMemory(device, buffer, bufferMemory, 0);
	
	return true;
}

void uploadBufferData(VulkanRenderDevice& vkDev, const VkDeviceMemory& bufferMemory, VkDeviceSize deviceOffset, const void* data, const size_t dataSize)
{
	
	
	void* mappedData = nullptr;
	vkMapMemory(vkDev.device, bufferMemory, deviceOffset, dataSize, 0, &mappedData);
	memcpy(mappedData, data, dataSize);
	vkUnmapMemory(vkDev.device, bufferMemory);
}

void downloadBufferData(VulkanRenderDevice& vkDev, const VkDeviceMemory& bufferMemory, VkDeviceSize deviceOffset, void* outData, const size_t dataSize)
{
	void* mappedData = nullptr;
	vkMapMemory(vkDev.device, bufferMemory, deviceOffset, dataSize, 0, &mappedData);
	memcpy(outData, mappedData, dataSize);
	vkUnmapMemory(vkDev.device, bufferMemory);
}

bool createUniformBuffer(VulkanRenderDevice& vkDev, VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkDeviceSize bufferSize)
{
	return createBuffer(vkDev.device, vkDev.physicalDevice, bufferSize,
						VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						buffer, bufferMemory);
}

void copyBuffer(VulkanRenderDevice& vkDev, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(vkDev);
	
	const VkBufferCopy copyRegion = {
		.srcOffset = 0,
		.dstOffset = 0,
		.size = size
	};
	
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	
	endSingleTimeCommands(vkDev, commandBuffer);
}

size_t allocateVertexBuffer(VulkanRenderDevice& vkDev, VkBuffer* storageBuffer, VkDeviceMemory* storageBufferMemory, size_t vertexDataSize, const void* vertexData, size_t indexDataSize, const void* indexData)
{
	VkDeviceSize bufferSize = vertexDataSize + indexDataSize;
	
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(vkDev.device, vkDev.physicalDevice, bufferSize,
				 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				 stagingBuffer, stagingBufferMemory);
	
	void* data;
	vkMapMemory(vkDev.device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertexData, vertexDataSize);
	memcpy((unsigned char *)data + vertexDataSize, indexData, indexDataSize);
	vkUnmapMemory(vkDev.device, stagingBufferMemory);
	
	createBuffer(vkDev.device, vkDev.physicalDevice, bufferSize,
				 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *storageBuffer, *storageBufferMemory);
	
	copyBuffer(vkDev, stagingBuffer, *storageBuffer, bufferSize);
	
	vkDestroyBuffer(vkDev.device, stagingBuffer, nullptr);
	vkFreeMemory(vkDev.device, stagingBufferMemory, nullptr);
	
	return bufferSize;
}

bool createTexturedVertexBuffer(VulkanRenderDevice& vkDev, const char* filename, VkBuffer* storageBuffer, VkDeviceMemory* storageBufferMemory, size_t* vertexBufferSize, size_t* indexBufferSize)
{
	const aiScene* scene = aiImportFile( filename, aiProcess_Triangulate );
	
	if ( !scene || !scene->HasMeshes() )
	{
		printf( "Unable to load %s\n", filename );
		exit( 255 );
	}
	
	const aiMesh* mesh = scene->mMeshes[0];
	struct VertexData
	{
		vec3 pos;
		vec2 tc;
	};
	
	std::vector<VertexData> vertices;
	for (unsigned i = 0; i != mesh->mNumVertices; i++)
	{
		const aiVector3D v = mesh->mVertices[i];
		const aiVector3D t = mesh->mTextureCoords[0][i];
		vertices.push_back({ .pos = vec3(v.x, v.z, v.y), .tc = vec2(t.x, 1.0f-t.y) });
	}
	
	std::vector<unsigned int> indices;
	for ( unsigned i = 0; i != mesh->mNumFaces; i++ )
	{
		for ( unsigned j = 0; j != 3; j++ )
			indices.push_back( mesh->mFaces[i].mIndices[j] );
	}
	aiReleaseImport( scene );
	
	*vertexBufferSize = sizeof(VertexData) * vertices.size();
	*indexBufferSize = sizeof(unsigned int) * indices.size();
	
	allocateVertexBuffer(vkDev, storageBuffer, storageBufferMemory, *vertexBufferSize, vertices.data(), *indexBufferSize, indices.data());
	
	return true;
}

