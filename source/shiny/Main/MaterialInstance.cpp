#include "MaterialInstance.hpp"

#include <stdexcept>

#include "Factory.hpp"
#include "ShaderSet.hpp"

namespace sh
{
	MaterialInstance::MaterialInstance (const std::string& name, Factory* f)
		: mName(name)
		, mShadersEnabled(true)
		, mFactory(f)
		, mListener(NULL)
	{
	}

	MaterialInstance::~MaterialInstance ()
	{
	}

	void MaterialInstance::setParentInstance (const std::string& name)
	{
		mParentInstance = name;
	}

	std::string MaterialInstance::getParentInstance ()
	{
		return mParentInstance;
	}

	void MaterialInstance::create (Platform* platform)
	{
		mMaterial = platform->createMaterial(mName);

		if (hasProperty ("shadow_caster_material"))
			mMaterial->setShadowCasterMaterial (retrieveValue<StringValue>(getProperty("shadow_caster_material"), NULL).get());

		if (hasProperty ("lod_values"))
			mMaterial->setLodLevels (retrieveValue<StringValue>(getProperty("lod_values"), NULL).get());
	}

	void MaterialInstance::destroyAll ()
	{
		if (hasProperty("create_configuration"))
			return;
		mMaterial->removeAll();
		mTexUnits.clear();
	}

	void MaterialInstance::setProperty (const std::string& name, PropertyValuePtr value)
	{
		PropertySetGet::setProperty (name, value);
		destroyAll(); // trigger updates
	}

	void MaterialInstance::createForConfiguration (const std::string& configuration, unsigned short lodIndex)
	{
		bool res = mMaterial->createConfiguration(configuration, lodIndex);
		if (!res)
			return; // listener was false positive

		if (mListener)
			mListener->requestedConfiguration (this, configuration);

		mFactory->setActiveConfiguration (configuration);
		mFactory->setActiveLodLevel (lodIndex);

		bool allowFixedFunction = true;
		if (!mShadersEnabled && hasProperty("allow_fixed_function"))
		{
			allowFixedFunction = retrieveValue<BooleanValue>(getProperty("allow_fixed_function"), NULL).get();
		}

		bool useShaders = mShadersEnabled || !allowFixedFunction;

		// get passes of the top-most parent
		PassVector passes = getPasses();
		if (passes.size() == 0)
			throw std::runtime_error ("material \"" + mName + "\" does not have any passes");

		for (PassVector::iterator it = passes.begin(); it != passes.end(); ++it)
		{
			boost::shared_ptr<Pass> pass = mMaterial->createPass (configuration, lodIndex);
			it->copyAll (pass.get(), this);

			// texture samplers used in the shaders
			std::vector<std::string> usedTextureSamplersVertex;
			std::vector<std::string> usedTextureSamplersFragment;

			PropertySetGet* context = this;

			// create or retrieve shaders
			bool hasVertex = it->hasProperty("vertex_program");
			bool hasFragment = it->hasProperty("fragment_program");
			if (useShaders)
			{
				it->setContext(context);
				it->mShaderProperties.setContext(context);
				if (hasVertex)
				{
					ShaderSet* vertex = mFactory->getShaderSet(retrieveValue<StringValue>(it->getProperty("vertex_program"), context).get());
					ShaderInstance* v = vertex->getInstance(&it->mShaderProperties);
					if (v)
					{
						pass->assignProgram (GPT_Vertex, v->getName());
						v->setUniformParameters (pass, &it->mShaderProperties);

						std::vector<std::string> sharedParams = v->getSharedParameters ();
						for (std::vector<std::string>::iterator it = sharedParams.begin(); it != sharedParams.end(); ++it)
						{
							pass->addSharedParameter (GPT_Vertex, *it);
						}

						std::vector<std::string> vector = v->getUsedSamplers ();
						usedTextureSamplersVertex.insert(usedTextureSamplersVertex.end(), vector.begin(), vector.end());
					}
				}
				if (hasFragment)
				{
					ShaderSet* fragment = mFactory->getShaderSet(retrieveValue<StringValue>(it->getProperty("fragment_program"), context).get());
					ShaderInstance* f = fragment->getInstance(&it->mShaderProperties);
					if (f)
					{
						pass->assignProgram (GPT_Fragment, f->getName());
						f->setUniformParameters (pass, &it->mShaderProperties);

						std::vector<std::string> sharedParams = f->getSharedParameters ();
						for (std::vector<std::string>::iterator it = sharedParams.begin(); it != sharedParams.end(); ++it)
						{
							pass->addSharedParameter (GPT_Fragment, *it);
						}

						std::vector<std::string> vector = f->getUsedSamplers ();
						usedTextureSamplersFragment.insert(usedTextureSamplersFragment.end(), vector.begin(), vector.end());
					}
				}
			}

			// create texture units
			std::vector<MaterialInstanceTextureUnit> texUnits = it->getTexUnits();
			int i=0;
			for (std::vector<MaterialInstanceTextureUnit>::iterator texIt = texUnits.begin(); texIt  != texUnits.end(); ++texIt )
			{
				// only create those that are needed by the shader, OR those marked to be created in fixed function pipeline if shaders are disabled
				bool foundVertex = std::find(usedTextureSamplersVertex.begin(), usedTextureSamplersVertex.end(), texIt->getName()) != usedTextureSamplersVertex.end();
				bool foundFragment = std::find(usedTextureSamplersFragment.begin(), usedTextureSamplersFragment.end(), texIt->getName()) != usedTextureSamplersFragment.end();
				if (  (foundVertex || foundFragment)
						|| (((!useShaders || (!hasVertex || !hasFragment)) && allowFixedFunction) && texIt->hasProperty("create_in_ffp") && retrieveValue<BooleanValue>(texIt->getProperty("create_in_ffp"), this).get()))
				{
					boost::shared_ptr<TextureUnitState> texUnit = pass->createTextureUnitState ();
					texIt->copyAll (texUnit.get(), context);

					mTexUnits.push_back(texUnit);

					// set texture unit indices (required by GLSL)
					if (useShaders && ((hasVertex && foundVertex) || (hasFragment && foundFragment)) && mFactory->getCurrentLanguage () == Language_GLSL)
					{
						pass->setTextureUnitIndex (foundVertex ? GPT_Vertex : GPT_Fragment, texIt->getName(), i);

						++i;
					}
				}
			}
		}

		if (mListener)
			mListener->createdConfiguration (this, configuration);
	}

	Material* MaterialInstance::getMaterial ()
	{
		return mMaterial.get();
	}

	MaterialInstancePass* MaterialInstance::createPass ()
	{
		mPasses.push_back (MaterialInstancePass());
		mPasses.back().setContext(this);
		return &mPasses.back();
	}

	PassVector MaterialInstance::getPasses()
	{
		if (mParent)
			return static_cast<MaterialInstance*>(mParent)->getPasses();
		else
			return mPasses;
	}

	void MaterialInstance::setShadersEnabled (bool enabled)
	{
		if (enabled == mShadersEnabled)
			return;
		mShadersEnabled = enabled;

		// trigger updates
		if (mMaterial.get())
			destroyAll();
	}

	void MaterialInstance::save (std::ofstream& stream)
	{
		stream << "material " << mName << "\n"
			   << "{\n";

		if (mParent)
		{
			stream << "\t" << "parent " << static_cast<MaterialInstance*>(mParent)->getName() << "\n";
		}

		const PropertyMap& properties = listProperties ();
		for (PropertyMap::const_iterator it = properties.begin(); it != properties.end(); ++it)
		{
			stream << "\t" << it->first << " " << retrieveValue<StringValue>(getProperty(it->first), NULL).get() << "\n";
		}

		for (PassVector::iterator it = mPasses.begin(); it != mPasses.end(); ++it)
		{
			stream << "\tpass" << '\n';
			stream << "\t{" << '\n';
			it->save(stream);
			stream << "\t}" << '\n';
		}

		stream << "}\n";
	}
}
