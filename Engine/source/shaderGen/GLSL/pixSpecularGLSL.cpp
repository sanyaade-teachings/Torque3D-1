//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "shaderGen/GLSL/pixSpecularGLSL.h"
#include "materials/processedMaterial.h"
#include "materials/materialFeatureTypes.h"
#include "shaderGen/shaderOp.h"
#include "shaderGen/shaderGenVars.h"
#include "gfx/gfxStructs.h"
#include "shaderGen/shaderGen.h"

void PBRConfigMapGLSL::processVert(Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd)
{
   MultiLine *meta = new MultiLine;

   // Add the texture coords.
   getOutTexCoord("texCoord",
     "vec2",
      fd.features[MFT_TexAnim],
      meta,
      componentList);

   output = meta;
}

void PBRConfigMapGLSL::processPix( Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd )
{
   // Get the texture coord.
   Var *texCoord = getInTexCoord( "texCoord", "vec2", componentList );

   // create texture var
   Var *pbrConfigMap = new Var;
   pbrConfigMap->setType( "sampler2D" );
   pbrConfigMap->setName( "PBRConfigMap" );
   pbrConfigMap->uniform = true;
   pbrConfigMap->sampler = true;
   pbrConfigMap->constNum = Var::getTexUnitNum();
   LangElement *texOp = new GenOp( "texture(@, @)", pbrConfigMap, texCoord );

   Var * pbrConfig = new Var( "PBRConfig", "vec4" );
   Var *metalness = (Var*)LangElement::find("metalness");
   if (!metalness) metalness = new Var("metalness", "float");
   Var *smoothness = (Var*)LangElement::find("smoothness");
   if (!smoothness) smoothness = new Var("smoothness", "float");
   MultiLine * meta = new MultiLine;

   meta->addStatement(new GenOp("   @ = @.r;\r\n", new DecOp(smoothness), texOp));
   meta->addStatement(new GenOp("   @ = @.b;\r\n", new DecOp(metalness), texOp));

   if (fd.features[MFT_InvertSmoothness])
      meta->addStatement(new GenOp("   @ = 1.0-@;\r\n", smoothness, smoothness));

   meta->addStatement(new GenOp("   @ = @.ggga;\r\n", new DecOp(pbrConfig), texOp));
   output = meta;
}

ShaderFeature::Resources PBRConfigMapGLSL::getResources( const MaterialFeatureData &fd )
{
   Resources res;
   res.numTex = 1;
   return res;
}

void PBRConfigMapGLSL::setTexData( Material::StageData &stageDat,
                                 const MaterialFeatureData &fd,
                                 RenderPassData &passData,
                                 U32 &texIndex )
{
   GFXTextureObject *tex = stageDat.getTex( MFT_PBRConfigMap );
   if ( tex )
   {
      passData.mTexType[ texIndex ] = Material::Standard;
      passData.mSamplerNames[ texIndex ] = "PBRConfigMap";
      passData.mTexSlot[ texIndex++ ].texObject = tex;
   }
}
