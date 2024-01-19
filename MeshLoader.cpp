// This has been adapted from the Vulkan tutorial

#include "Starter.hpp"
#include "Camera.hpp"
#include "GameLogic.hpp"

// The uniform buffer objects data structures
// Remember to use the correct alignas(...) value
//        float : alignas(4)
//        vec2  : alignas(8)
//        vec3  : alignas(16)
//        vec4  : alignas(16)
//        mat3  : alignas(16)
//        mat4  : alignas(16)
// Example:
struct UniformBlock {
	alignas(16) glm::mat4 mvpMat;
};

// The vertices data structures
// Example
struct Vertex {
	glm::vec3 pos;
	glm::vec2 UV;
};

struct BallObject {
    Model<Vertex> model;
    DescriptorSet descriptorSet;
    UniformBlock ubo;
    glm::vec3 pos;
};

// MAIN ! 
class MeshLoader : public BaseProject {
protected:
    
    // Descriptor Layouts ["classes" of what will be passed to the shaders]
    DescriptorSetLayout DSL;
    
    // Vertex formats
    VertexDescriptor VD;
    
    // Pipelines [Shader couples]
    Pipeline P;
    
    // Models, textures and Descriptors (values assigned to the uniforms)
    // Please note that Model objects depends on the corresponding vertex structure
    // Models
    Model<Vertex> M1, M2, M3, M4, MTable;
    // Descriptor sets
    DescriptorSet DS1, DS2, DS3, DS4, DSTable;
    // Textures
    Texture T1, T2, TFurniture;
    
    // C++ storage for uniform variables
    UniformBlock ubo1, ubo2, ubo3, ubo4, uboTable;
    
    // Other application parameters
    BallObject balls[NUM_BALLS];
    Camera camera;
    GameLogic gameLogic;
    // Here you set the main application parameters
    void setWindowParameters() {
        // window size, titile and initial background
        windowWidth = 800;
        windowHeight = 600;
        windowTitle = "Mesh Loader";
        windowResizable = GLFW_TRUE;
        initialBackgroundColor = {0.0f, 0.005f, 0.01f, 1.0f};
        
        // Descriptor pool sizes
        uniformBlocksInPool = 5 + NUM_BALLS;
        texturesInPool = 6 + NUM_BALLS;
        setsInPool = 5 + NUM_BALLS;
        
        camera.aspectRatio = (float)windowWidth / (float)windowHeight;
    }
    
    // What to do when the window changes size
    void onWindowResize(int w, int h) {
        camera.aspectRatio = (float)w / (float)h;
    }
    
    // Here you load and setup all your Vulkan Models and Texutures.
    // Here you also create your Descriptor set layouts and load the shaders for the pipelines
    void localInit() {
        // Descriptor Layouts [what will be passed to the shaders]
        DSL.init(this, {
            // this array contains the bindings:
            // first  element : the binding number
            // second element : the type of element (buffer or texture)
            //                  using the corresponding Vulkan constant
            // third  element : the pipeline stage where it will be used
            //                  using the corresponding Vulkan constant
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
            {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
        });
        
        // Vertex descriptors
        VD.init(this, {
            // this array contains the bindings
            // first  element : the binding number
            // second element : the stride of this binging
            // third  element : whether this parameter change per vertex or per instance
            //                  using the corresponding Vulkan constant
            {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}
        }, {
            // this array contains the location
            // first  element : the binding number
            // second element : the location number
            // third  element : the offset of this element in the memory record
            // fourth element : the data type of the element
            //                  using the corresponding Vulkan constant
            // fifth  elmenet : the size in byte of the element
            // sixth  element : a constant defining the element usage
            //                   POSITION - a vec3 with the position
            //                   NORMAL   - a vec3 with the normal vector
            //                   UV       - a vec2 with a UV coordinate
            //                   COLOR    - a vec4 with a RGBA color
            //                   TANGENT  - a vec4 with the tangent vector
            //                   OTHER    - anything else
            //
            // ***************** DOUBLE CHECK ********************
            //    That the Vertex data structure you use in the "offsetoff" and
            //	in the "sizeof" in the previous array, refers to the correct one,
            //	if you have more than one vertex format!
            // ***************************************************
            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos),
                sizeof(glm::vec3), POSITION},
            {0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, UV),
                sizeof(glm::vec2), UV}
        });
        
        // Pipelines [Shader couples]
        // The second parameter is the pointer to the vertex definition
        // Third and fourth parameters are respectively the vertex and fragment shaders
        // The last array, is a vector of pointer to the layouts of the sets that will
        // be used in this pipeline. The first element will be set 0, and so on..
        P.init(this, &VD, "shaders/ShaderVert.spv", "shaders/ShaderFrag.spv", {&DSL});
        
        // Models, textures and Descriptors (values assigned to the uniforms)
        
        // Create models
        // The second parameter is the pointer to the vertex definition for this model
        // The third parameter is the file name
        // The last is a constant specifying the file type: currently only OBJ or GLTF
        M1.init(this,   &VD, "Models/Cube.obj", OBJ);
        M2.init(this,   &VD, "Models/Sphere.gltf", GLTF);
        M3.init(this,   &VD, "Models/dish.005_Mesh.098.mgcg", MGCG);
        
        // Creates a mesh with direct enumeration of vertices and indices
        M4.vertices = {{{-6,-2,-6}, {0.0f,0.0f}}, {{-6,-2,6}, {0.0f,1.0f}},
            {{6,-2,-6}, {1.0f,0.0f}}, {{ 6,-2,6}, {1.0f,1.0f}}};
        M4.indices = {0, 1, 2,    1, 3, 2};
        M4.initMesh(this, &VD);
        
        MTable.init(this, &VD, "models/coffee_table_019_Mesh.6459.mgcg", MGCG);
        for (auto &ball : balls) {
            ball.model.init(this, &VD, "Models/Sphere.gltf", GLTF);
        }
        
        // Create the textures
        // The second parameter is the file name
        T1.init(this,   "textures/Checker.png");
        T2.init(this,   "textures/Textures_Food.png");
        TFurniture.init(this, "textures/Textures_Forniture.png");
        
        // Init local variables
        initCamera(camera);
        gameLogic.initBalls();
    }
    
	// Here you create your pipelines and Descriptor Sets!
	void pipelinesAndDescriptorSetsInit() {
		// This creates a new pipeline (with the current surface), using its shaders
		P.create();

		// Here you define the data set
		DS1.init(this, &DSL, {
		// the second parameter, is a pointer to the Uniform Set Layout of this set
		// the last parameter is an array, with one element per binding of the set.
		// first  elmenet : the binding number
		// second element : UNIFORM or TEXTURE (an enum) depending on the type
		// third  element : only for UNIFORMs, the size of the corresponding C++ object. For texture, just put 0
		// fourth element : only for TEXTUREs, the pointer to the corresponding texture object. For uniforms, use nullptr
					{0, UNIFORM, sizeof(UniformBlock), nullptr},
					{1, TEXTURE, 0, &T1}
				});
		DS2.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBlock), nullptr},
					{1, TEXTURE, 0, &T1}
				});
		DS3.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBlock), nullptr},
					{1, TEXTURE, 0, &T2}
				});
		DS4.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBlock), nullptr},
					{1, TEXTURE, 0, &T1}
				});
        DSTable.init(this, &DSL, {
            {0, UNIFORM, sizeof(UniformBlock), nullptr},
            {1, TEXTURE, 0, &TFurniture}
        });
        
        for(auto &ball : balls) {
            ball.descriptorSet.init(this, &DSL, {
                {0, UNIFORM, sizeof(UniformBlock), nullptr},
                {1, TEXTURE, 0, &T2}
            });
        }
	}

	// Here you destroy your pipelines and Descriptor Sets!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	void pipelinesAndDescriptorSetsCleanup() {
		// Cleanup pipelines
		P.cleanup();

		// Cleanup datasets
		DS1.cleanup();
		DS2.cleanup();
		DS3.cleanup();
		DS4.cleanup();
        DSTable.cleanup();
        
        for(auto &ball : balls) {
            ball.descriptorSet.cleanup();
        }
	}

	// Here you destroy all the Models, Texture and Desc. Set Layouts you created!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	// You also have to destroy the pipelines: since they need to be rebuilt, they have two different
	// methods: .cleanup() recreates them, while .destroy() delete them completely
	void localCleanup() {
		// Cleanup textures
		T1.cleanup();
		T2.cleanup();
		
		// Cleanup models
		M1.cleanup();
		M2.cleanup();
		M3.cleanup();
		M4.cleanup();
        MTable.cleanup();
        
        for(auto &ball : balls) {
            ball.model.cleanup();
        }
		
		// Cleanup descriptor set layouts
		DSL.cleanup();
		
		// Destroies the pipelines
		P.destroy();		
	}
	
	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures
	
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
		// binds the pipeline
		P.bind(commandBuffer);
		// For a pipeline object, this command binds the corresponing pipeline to the command buffer passed in its parameter

		// binds the data set
		DS1.bind(commandBuffer, P, 0, currentImage);
		// For a Dataset object, this command binds the corresponing dataset
		// to the command buffer and pipeline passed in its first and second parameters.
		// The third parameter is the number of the set being bound
		// As described in the Vulkan tutorial, a different dataset is required for each image in the swap chain.
		// This is done automatically in file Starter.hpp, however the command here needs also the index
		// of the current image in the swap chain, passed in its last parameter
					
		// binds the model
		M1.bind(commandBuffer);
		// For a Model object, this command binds the corresponing index and vertex buffer
		// to the command buffer passed in its parameter
		
		// record the drawing command in the command buffer
		vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(M1.indices.size()), 1, 0, 0, 0);
		// the second parameter is the number of indexes to be drawn. For a Model object,
		// this can be retrieved with the .indices.size() method.

		DS2.bind(commandBuffer, P, 0, currentImage);
		M2.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(M2.indices.size()), 1, 0, 0, 0);
		DS3.bind(commandBuffer, P, 0, currentImage);
		M3.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(M3.indices.size()), 1, 0, 0, 0);
		DS4.bind(commandBuffer, P, 0, currentImage);
		M4.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(M4.indices.size()), 1, 0, 0, 0);
        
        DSTable.bind(commandBuffer, P, 0, currentImage);
        MTable.bind(commandBuffer);
        vkCmdDrawIndexed(commandBuffer,static_cast<uint32_t>(MTable.indices.size()), 1, 0, 0, 0);
        
        for(auto &ball : balls) {
            ball.descriptorSet.bind(commandBuffer, P, 0, currentImage);
            ball.model.bind(commandBuffer);
            vkCmdDrawIndexed(commandBuffer,static_cast<uint32_t>(ball.model.indices.size()), 1, 0, 0, 0);
        }
	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		// Standard procedure to quit when the ESC key is pressed
		if(glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
        
        Input input;
        getSixAxis(input.deltaT, input.m, input.r, input.fire);
        
        updateCamera(camera, input);
		// getSixAxis() is defined in Starter.hpp in the base class.
		// It fills the float point variable passed in its first parameter with the time
		// since the last call to the procedure.
		// It fills vec3 in the second parameters, with three values in the -1,1 range corresponding
		// to motion (with left stick of the gamepad, or ASWD + RF keys on the keyboard)
		// It fills vec3 in the third parameters, with three values in the -1,1 range corresponding
		// to motion (with right stick of the gamepad, or Arrow keys + QE keys on the keyboard, or mouse)
		// If fills the last boolean variable with true if fire has been pressed:
		//          SPACE on the keyboard, A or B button on the Gamepad, Right mouse button

        glm::mat4 ViewProjection = computeViewProjectionMatrix(camera);
        
		glm::mat4 World;

		World = glm::translate(glm::mat4(1), glm::vec3(-3, 0, 0)); // cube
		ubo1.mvpMat = ViewProjection * World;
		DS1.map(currentImage, &ubo1, sizeof(ubo1), 0);
		// the .map() method of a DataSet object, requires the current image of the swap chain as first parameter
		// the second parameter is the pointer to the C++ data structure to transfer to the GPU
		// the third parameter is its size
		// the fourth parameter is the location inside the descriptor set of this uniform block
		World = glm::translate(glm::mat4(1), glm::vec3(3, 0, 0)); // sphere
		ubo2.mvpMat = ViewProjection * World;
		DS2.map(currentImage, &ubo2, sizeof(ubo2), 0);
		
		World = glm::scale(glm::mat4(1), glm::vec3(10.0f)); // dish
		ubo3.mvpMat = ViewProjection * World;
		DS3.map(currentImage, &ubo3, sizeof(ubo3), 0);

		World = glm::translate(glm::mat4(1), glm::vec3(0, -5, 0)) * // rectangle
				glm::scale(glm::mat4(1), glm::vec3(5.0f));
		ubo4.mvpMat = ViewProjection * World;
		DS4.map(currentImage, &ubo4, sizeof(ubo4), 0);
        
        World = glm::translate(glm::mat4(1), glm::vec3(0, 0, 0)) * // rectangle
                glm::scale(glm::mat4(1), glm::vec3(5.0f));
        uboTable.mvpMat = ViewProjection * World;
        DSTable.map(currentImage, &uboTable, sizeof(uboTable), 0);
        
        for (int i = 0; i < NUM_BALLS; i++) {
            BallObject &obj = balls[i];
            obj.ubo.mvpMat = ViewProjection * gameLogic.getBall(i).computeWorldMatrix();
            obj.descriptorSet.map(currentImage, &obj.ubo, sizeof(obj.ubo), 0);
        }
	}
};


// This is the main: probably you do not need to touch this!
int main() {
    MeshLoader app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
