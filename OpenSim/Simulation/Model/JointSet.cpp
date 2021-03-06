/* -------------------------------------------------------------------------- *
 *                           OpenSim:  JointSet.cpp                           *
 * -------------------------------------------------------------------------- *
 * The OpenSim API is a toolkit for musculoskeletal modeling and simulation.  *
 * See http://opensim.stanford.edu and the NOTICE file for more information.  *
 * OpenSim is developed at Stanford University and supported by the US        *
 * National Institutes of Health (U54 GM072970, R24 HD065690) and by DARPA    *
 * through the Warrior Web program.                                           *
 *                                                                            *
 * Copyright (c) 2005-2012 Stanford University and the Authors                *
 * Author(s): Peter Loan                                                      *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may    *
 * not use this file except in compliance with the License. You may obtain a  *
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0.         *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */

#include "JointSet.h"
#include <OpenSim/Simulation/Model/Model.h>
#include <OpenSim/Simulation/Model/BodySet.h>
#include <OpenSim/Common/ScaleSet.h>

using namespace std;
using namespace OpenSim;

//=============================================================================
// DESTRUCTOR AND CONSTRUCTORS
//=============================================================================
//_____________________________________________________________________________
/**
 * Destructor.
 */
JointSet::~JointSet(void)
{
}

//_____________________________________________________________________________
/**
 * Default constructor of a JointSet.
 */
JointSet::JointSet()
{
	setNull();
}

JointSet::JointSet(Model& model) :
	ModelComponentSet<Joint>(model)
{
	setNull();
}

//_____________________________________________________________________________
/**
 * Copy constructor of a JointSet.
 */
JointSet::JointSet(const JointSet& aJointSet):
	ModelComponentSet<Joint>(aJointSet)
{
	setNull();
	*this = aJointSet;
}

//=============================================================================
// CONSTRUCTION METHODS
//=============================================================================
/**
 * Set the data members of this JointSet to their null values.
 */
void JointSet::setNull()
{
}

/**
 * This ModelComponentSet method is overridden to ensure that joints are 
 * processed from ground outward.
 */
void JointSet::invokeAddToSystem(SimTK::MultibodySystem& system) const
{
    vector<bool> hasProcessed(getSize(), false);
    map<Body*, int> bodyMap;
    for (int i = 0; i < getSize(); i++)
    {
        Joint& joint = get(i);
        bodyMap[&joint.updBody()] = i;
    }
	for (int i = 0; i < getSize(); i++){
		if (getDebugLevel()>=2) cout << "Calling addToSystem for Joint " << get(i).getName() << " ..." << endl;
        addToSystemForOneJoint(system, i, bodyMap, hasProcessed);

	}
}

void JointSet::addToSystemForOneJoint(SimTK::MultibodySystem& system, int jointIndex, const map<Body*, int>& bodyMap, vector<bool>& hasProcessed) const
{
    if (hasProcessed[jointIndex])
        return;
    hasProcessed[jointIndex] = true;
    Joint& joint = get(jointIndex);

    // Make sure the parent joint is processed first.

    map<Body*, int>::const_iterator parent = bodyMap.find(&joint.updParentBody());
    if (parent != bodyMap.end())
    {
        int parentIndex = parent->second;
        if (!hasProcessed[parentIndex])
            addToSystemForOneJoint(system, parentIndex, bodyMap, hasProcessed);
    }

    static_cast<const Joint&>(get(jointIndex)).addToSystem(system);
}

/**
 * Populate the a flat list of Joints given a Model that has been setup
 */
void JointSet::populate(Model& aModel)
{
    setMemoryOwner(false);
    setSize(0);

    for(int i=0; i< aModel.getNumBodies(); i++){
        if (aModel.getBodySet().get(i).hasJoint()) { // Ground body doesn't have a jnt
            Joint& nextJoint = aModel.getBodySet().get(i).getJoint();
			nextJoint.setBody(aModel.getBodySet().get(i));
            adoptAndAppend(&nextJoint);
        }
    }
}

//=============================================================================
// OPERATORS
//=============================================================================
//_____________________________________________________________________________
/**
 * Assignment operator.
 *
 * @return Reference to this object.
 */
#ifndef SWIG
JointSet& JointSet::operator=(const JointSet &aJointSet)
{
	Set<Joint>::operator=(aJointSet);
	return (*this);
}
#endif

//=============================================================================
// UTILITY
//=============================================================================
//_____________________________________________________________________________
/**
 * Scale joint set by a set of scale factors
 */
void JointSet::scale(const ScaleSet& aScaleSet)
{
	for(int i=0; i<getSize(); i++) get(i).scale(aScaleSet);
}
