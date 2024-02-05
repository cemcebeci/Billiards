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
    alignas(16) glm::mat4 wMat;
    alignas(16) glm::mat4 nMat = glm::mat4(1);
};

struct OverlayUniformBlock {
    alignas(4) float visible;
};

struct SpotlightUniformBufferObject {
    alignas(16) glm::vec3 lightPos;
    alignas(16) glm::vec3 lightDir;
    alignas(16) glm::vec4 lightColor;
    alignas(16) glm::vec3 eyePos;
};


// The vertices data structures
// Example
struct Vertex {
	glm::vec3 pos;
    glm::vec3 norm;
	glm::vec2 UV;
};

struct VertexOverlay {
    glm::vec2 pos;
    glm::vec2 UV;
};


struct BallObject {
    Model<Vertex> model;
    DescriptorSet descriptorSet;
    UniformBlock ubo;
    glm::vec3 pos;
    Texture tex;
};

// MAIN ! 
class MeshLoader : public BaseProject {
protected:
    
    // Descriptor Layouts ["classes" of what will be passed to the shaders]
    DescriptorSetLayout DSL, DSLLighting;
    
    // Vertex formats
    VertexDescriptor VD;
    VertexDescriptor VOverlay;
    
    // Pipelines [Shader couples]
    Pipeline PBlinn;
    Pipeline POrenNayar;
    Pipeline POverlay;
    
    // Models, textures and Descriptors (values assigned to the uniforms)
    // Please note that Model objects depends on the corresponding vertex structure
    // Models
    Model<Vertex> MTable, MStick, MPointer;
    Model<VertexOverlay> MP1Turn, MP2Turn;
    // Descriptor sets
    DescriptorSet DSTable, DSStick, DSPointer, DSP1Turn, DSP2Turn, DSLighting;
    // Textures
    Texture TPointer, TFurniture, TP1Turn, TP2Turn, TStick;
    
    // C++ storage for uniform variables
    UniformBlock uboTable, uboStick, uboPointer;
    OverlayUniformBlock uboP1Turn, uboP2Turn;
    SpotlightUniformBufferObject uboLighting;
    
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
        uniformBlocksInPool = 10 + NUM_BALLS;
        texturesInPool = 11 + NUM_BALLS + NUM_BALLS;
        setsInPool = 10 + NUM_BALLS;
        
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
        
        DSLLighting.init(this , {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
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
            {0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, norm), sizeof(glm::vec3), NORMAL},
            {0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, UV),
                sizeof(glm::vec2), UV}
        });
        
        VOverlay.init(this, {
                  {0, sizeof(VertexOverlay), VK_VERTEX_INPUT_RATE_VERTEX}
                }, {
                  {0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexOverlay, pos),
                         sizeof(glm::vec2), OTHER},
                  {0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexOverlay, UV),
                         sizeof(glm::vec2), UV}
                });
        
        // Pipelines [Shader couples]
        // The second parameter is the pointer to the vertex definition
        // Third and fourth parameters are respectively the vertex and fragment shaders
        // The last array, is a vector of pointer to the layouts of the sets that will
        // be used in this pipeline. The first element will be set 0, and so on..
        PBlinn.init(this, &VD, "shaders/BlinnVert.spv", "shaders/BlinnFrag.spv", {&DSLLighting, &DSL});
        POrenNayar.init(this, &VD, "shaders/OrenNayarVert.spv", "shaders/OrenNayarFrag.spv", {&DSLLighting, &DSL});
        POverlay.init(this, &VOverlay, "shaders/OverlayVert.spv", "shaders/OverlayFrag.spv", {&DSL});
        POverlay.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
                                     VK_CULL_MODE_NONE, false);
        
        // Models, textures and Descriptors (values assigned to the uniforms)
        
        // Create models
        // The second parameter is the pointer to the vertex definition for this model
        // The third parameter is the file name
        // The last is a constant specifying the file type: currently only OBJ or GLTF
        
        MTable.init(this, &VD, "models/billiardtable-TurboSquid.obj", OBJ);
        for (auto &ball : balls) {
            ball.model.init(this, &VD, "Models/ball.obj", OBJ);
        }
        MStick.init(this, &VD, "models/stick.obj", OBJ);
        MPointer.init(this, &VD, "models/ball.obj", OBJ);
        
        auto margin = 0.05f;
        MP1Turn.vertices = {{{0 - margin, -1 + margin}, {0.0f, 0.0f}}, {{0 - margin, -0.82 + margin}, {0.0f,1.0f}},
                         {{ 1.0f - margin, -1 + margin}, {1.0f,0.0f}}, {{ 1.0f - margin, -0.82 + margin}, {1.0f,1.0f}}};
        MP1Turn.indices = {0, 1, 2,    1, 3, 2};
        MP1Turn.initMesh(this, &VD);
        
        MP2Turn.vertices = {{{0 - margin, -1 + margin}, {0.0f, 0.0f}}, {{0 - margin, -0.82 + margin}, {0.0f,1.0f}},
                         {{ 1.0f - margin, -1 + margin}, {1.0f,0.0f}}, {{ 1.0f - margin, -0.82 + margin}, {1.0f,1.0f}}};
        MP2Turn.indices = {0, 1, 2,    1, 3, 2};
        MP2Turn.initMesh(this, &VD);
        
        // Create the textures
        // The second parameter is the file name
        TPointer.init(this,   "textures/ball_8.png");
        TFurniture.init(this, "textures/Table.png");
        TStick.init(this, "textures/BilliardStick_DefaultMaterial_AlbedoTransparency.png");
        int id = 0;
        for (auto &ball : balls) {
            std::string path = "textures/ball_" + std::to_string(id++) + ".png";
            ball.tex.init(this, path.data());
        }
        TP1Turn.init(this,"textures/Player_1_turn.png");
        TP2Turn.init(this,"textures/Player_2_turn.png");

        
        // Init local variables
        initCamera(camera);
        gameLogic.init();
    }
    
	// Here you create your pipelines and Descriptor Sets!
	void pipelinesAndDescriptorSetsInit() {
		// This creates a new pipeline (with the current surface), using its shaders
		PBlinn.create();
        POrenNayar.create();
        POverlay.create();

		// Here you define the data set
        DSTable.init(this, &DSL, {
            {0, UNIFORM, sizeof(UniformBlock), nullptr},
            {1, TEXTURE, 0, &TFurniture}
        });
        DSStick.init(this, &DSL, {
            {0, UNIFORM, sizeof(UniformBlock), nullptr},
            {1, TEXTURE, 0, &TStick}
        });
        DSPointer.init(this, &DSL, {
            {0, UNIFORM, sizeof(UniformBlock), nullptr},
            {1, TEXTURE, 0, &TPointer}
        });
        DSP1Turn.init(this, &DSL, {
            {0, UNIFORM, sizeof(OverlayUniformBlock), nullptr},
            {1, TEXTURE, 0, &TP1Turn}
        });
        DSP2Turn.init(this, &DSL, {
            {0, UNIFORM, sizeof(OverlayUniformBlock), nullptr},
            {1, TEXTURE, 0, &TP2Turn}
        });
        
        for(auto &ball : balls) {
            ball.descriptorSet.init(this, &DSL, {
                {0, UNIFORM, sizeof(UniformBlock), nullptr},
                {1, TEXTURE, 0, &ball.tex}
            });
        }
        
        DSLighting.init(this, &DSLLighting, {
            {0, UNIFORM, sizeof(SpotlightUniformBufferObject), nullptr}
        });
	}

	// Here you destroy your pipelines and Descriptor Sets!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	void pipelinesAndDescriptorSetsCleanup() {
		// Cleanup pipelines
		PBlinn.cleanup();
        POrenNayar.cleanup();
        POverlay.cleanup();

		// Cleanup datasets
        DSTable.cleanup();
        DSStick.cleanup();
        DSPointer.cleanup();
        DSP1Turn.cleanup();
        DSP2Turn.cleanup();
        DSLighting.cleanup();
        
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
		TPointer.cleanup();
        TFurniture.cleanup();
        TP1Turn.cleanup();
        TP2Turn.cleanup();
        TStick.cleanup();
		
		// Cleanup models
        MTable.cleanup();
        MStick.cleanup();
        MPointer.cleanup();
        MP1Turn.cleanup();
        MP2Turn.cleanup();
        
        for(auto &ball : balls) {
            ball.model.cleanup();
        }
		
		// Cleanup descriptor set layouts
		DSL.cleanup();
		
		// Destroies the pipelines
		PBlinn.destroy();
        POrenNayar.destroy();
        POverlay.destroy();
	}
	
	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures
	
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {

        DSLighting.bind(commandBuffer, PBlinn, 0, currentImage);
        // DSLighting.bind(commandBuffer, POrenNayar, 0, currentImage);
        
		// For a Dataset object, this command binds the corresponing dataset
		// to the command buffer and pipeline passed in its first and second parameters.
		// The third parameter is the number of the set being bound
		// As described in the Vulkan tutorial, a different dataset is required for each image in the swap chain.
		// This is done automatically in file Starter.hpp, however the command here needs also the index
		// of the current image in the swap chain, passed in its last parameter
					
		// For a Model object, this command binds the corresponing index and vertex buffer
		// to the command buffer passed in its parameter
		// the second parameter is the number of indexes to be drawn. For a Model object,
		// this can be retrieved with the .indices.size() method.
        POrenNayar.bind(commandBuffer);
        DSTable.bind(commandBuffer, PBlinn, 1, currentImage);
        MTable.bind(commandBuffer);
        vkCmdDrawIndexed(commandBuffer,static_cast<uint32_t>(MTable.indices.size()), 1, 0, 0, 0);
        
        PBlinn.bind(commandBuffer);
        DSStick.bind(commandBuffer, PBlinn, 1, currentImage);
        MStick.bind(commandBuffer);
        vkCmdDrawIndexed(commandBuffer,static_cast<uint32_t>(MStick.indices.size()), 1, 0, 0, 0);
        
        DSPointer.bind(commandBuffer, PBlinn, 1, currentImage);
        MPointer.bind(commandBuffer);
        vkCmdDrawIndexed(commandBuffer,static_cast<uint32_t>(MPointer.indices.size()), 1, 0, 0, 0);
        
        for(auto &ball : balls) {
            ball.descriptorSet.bind(commandBuffer, PBlinn, 1, currentImage);
            ball.model.bind(commandBuffer);
            vkCmdDrawIndexed(commandBuffer,static_cast<uint32_t>(ball.model.indices.size()), 1, 0, 0, 0);
        }
        
        POverlay.bind(commandBuffer);
        DSP1Turn.bind(commandBuffer, POverlay, 0, currentImage);
        MP1Turn.bind(commandBuffer);
        vkCmdDrawIndexed(commandBuffer,static_cast<uint32_t>(MP1Turn.indices.size()), 1, 0, 0, 0);
        
        DSP2Turn.bind(commandBuffer, POverlay, 0, currentImage);
        MP2Turn.bind(commandBuffer);
        vkCmdDrawIndexed(commandBuffer,static_cast<uint32_t>(MP2Turn.indices.size()), 1, 0, 0, 0);
        
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
        gameLogic.updateGame(input);
        
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

		// the .map() method of a DataSet object, requires the current image of the swap chain as first parameter
		// the second parameter is the pointer to the C++ data structure to transfer to the GPU
		// the third parameter is its size
		// the fourth parameter is the location inside the descriptor set of this uniform block
        World = glm::translate(glm::mat4(1), glm::vec3(0, -1, 0)) * // Table
                glm::scale(glm::mat4(1), glm::vec3(7.0f));
        uboTable.mvpMat = ViewProjection * World;
        uboTable.wMat = World;
        uboTable.nMat = glm::inverse(glm::transpose(World));
        DSTable.map(currentImage, &uboTable, sizeof(uboTable), 0);
        
        World = gameLogic.computeStickWorldMatrix() * glm::scale(glm::mat4(1), glm::vec3(2));
        uboStick.mvpMat = ViewProjection * World;
        uboStick.wMat = World;
        uboStick.nMat = glm::inverse(glm::transpose(World));
        DSStick.map(currentImage, &uboStick, sizeof(uboStick), 0);

        
        World = gameLogic.pointerWorldMatrix();
        uboPointer.mvpMat = ViewProjection * World;
        uboPointer.wMat = World;
        uboPointer.nMat = glm::inverse(glm::transpose(World));
        DSPointer.map(currentImage, &uboPointer, sizeof(uboPointer), 0);
        
        for (int i = 0; i < NUM_BALLS; i++) {
            BallObject &obj = balls[i];
            obj.ubo.mvpMat = ViewProjection * gameLogic.getBall(i).computeWorldMatrix();
            obj.ubo.nMat = glm::inverse(glm::transpose(/*viewMatrix(camera) * */ gameLogic.getBall(i).computeWorldMatrix()));
            obj.ubo.wMat = gameLogic.getBall(i).computeWorldMatrix();
            obj.descriptorSet.map(currentImage, &obj.ubo, sizeof(obj.ubo), 0);
        }
        
        uboP1Turn.visible = (gameLogic.getCurrentPlayer() == 0) ? 1.0f : 0.0f;
        DSP1Turn.map(currentImage, &uboP1Turn, sizeof(uboP1Turn), 0);
        
        uboP2Turn.visible = (gameLogic.getCurrentPlayer() == 1) ? 1.0f : 0.0f;
        DSP2Turn.map(currentImage, &uboP2Turn, sizeof(uboP2Turn), 0);
        
        uboLighting.lightPos = glm::vec3(0, 10, 0);
        uboLighting.lightColor = glm::vec4(1, 1, 1, 1);
        uboLighting.lightDir = glm::vec3(0,-1,0);
        uboLighting.eyePos = camera.position;
        DSLighting.map(currentImage, &uboLighting, sizeof(uboLighting), 0);
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
