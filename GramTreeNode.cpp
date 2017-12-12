#include "Scanner.h"
#include "GramTreeNode.h"

void GramTreeNodeBase:: addChild(GramTreeNodeBase* pChild, int ind )  {
	if (pChild)   {
		if (ind >= 0)  {
			child_[ind] = pChild;
		}
		pChild->_childIndex = ind;
		pChild->_pParent = this;
	}
}



string ClassTreeNode::getName()
{
	return "";
}

string SubroutineDecNode::getName()
{
	auto pNode = child_[SubroutineFiled::Name];
	return pNode->getLexeme();
}

string SubroutineDecNode::getSignName()
{
	auto pNode = child_[SubroutineFiled::Sign];
	return pNode->getLexeme();
}

int SubroutineDecNode::getFuncLocalsNum()
{
	return getSubroutineBody()->getFuncLocalsNum();
}

SubroutineBodyNode* SubroutineDecNode::getSubroutineBody()
{
	return (SubroutineBodyNode*)child_[SubroutineFiled::Body];
}




int SubroutineBodyNode::getFuncLocalsNum()
{
	int nlocals = 0;

	for (auto q = getChildByIndex(0); q != nullptr; q = q->getNextNode())  {
		nlocals++;
	}
	return nlocals;
}



GramTreeNodeBase* AssignStatement::getChildByTag(string name)
{
	if (name == "var_name")  {
		return child_[AssignLetf];
	}
	else if (name == "var_rval")  {
		return child_[AssignRight];
	}
	return nullptr;
}


