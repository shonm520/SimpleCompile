#include "Scanner.h"
#include "GramTreeNode.h"


stack<ClassTreeNode*> GramTreeNodeBase::s_stackCurClassZone;
stack<SubroutineDecNode*> GramTreeNodeBase::s_stackCurSubroutineZone;
stack<SubroutineBodyNode*> GramTreeNodeBase::s_stackCurSubroutineBodyZone;
stack<CompondStatement*> GramTreeNodeBase::s_stackCurCompoundStatmentZone;
int GramTreeNodeBase::s_nCurNodeIndex = 0;

void GramTreeNodeBase:: addChild(GramTreeNodeBase* pChild, int ind )  {
	if (pChild)   {
		if (ind >= 0)  {
			_child[ind] = pChild;
		}
		pChild->_childIndex = ind;
		int simbling = 0;
		for (auto p = pChild; p != nullptr; p = p->getNextNode())  {    //所有兄弟节点都要有父节点
			p->_pParent = this;
			simbling++;
		}
		pChild->_siblings = ((simbling == 0) ? 1 : simbling);     //大哥存有同等兄弟节点的个数,不是所有父节点孩子的个数
	}
}

string GramTreeNodeBase::getClassName()       //获得类名
{
	if (_strClassName == "")  {  //为了缓存,提高效率
		if (_nodeKind == CLASS_K)  {
			_strClassName = ((ClassTreeNode*)this)->getClassName();
		}
		else  {
			auto p = getParentNode();
			while (p->getNodeKind() != CLASS_K)  {    //向上找到最终的类名
				p = p->getParentNode();
			}
			_strClassName = ((ClassTreeNode*)p)->getClassName();
		}
	}
	return _strClassName;
}


ClassTreeNode* GramTreeNodeBase::getCurCurClassNode()
{
	if (s_stackCurClassZone.size() == 0)  {
		return nullptr;
	}
	return s_stackCurClassZone.top();
}

void GramTreeNodeBase::insertClassNode(ClassTreeNode* node)
{
	s_stackCurClassZone.push(node);
}

void GramTreeNodeBase::quitClassZone()
{
	assert(s_stackCurClassZone.size() > 0);
	s_stackCurClassZone.pop();
}




bool ClassTreeNode::hasVarDecInField(GramTreeNodeBase* node)
{
	auto field_var = _child[Class_VarDec];
	while (field_var)  {
		if (field_var->getChildByIndex(2)->getLexeme() == node->getLexeme())  {
			return true;
		}
		field_var = field_var->getNextNode();
	}
	return false;
}




SubroutineDecNode* GramTreeNodeBase::getCurSubroutineNode()
{
	if (s_stackCurSubroutineZone.size() == 0)  {
		return nullptr;
	}
	return s_stackCurSubroutineZone.top();
}

void GramTreeNodeBase::insertSubRoutineNode(SubroutineDecNode* node)
{
	s_stackCurSubroutineZone.push(node);
}

void GramTreeNodeBase::quitSubRoutineZone()
{
	assert(s_stackCurSubroutineZone.size() > 0);
	s_stackCurSubroutineZone.pop();
}



SubroutineBodyNode* GramTreeNodeBase::getCurSubroutineBodyNode()
{
	if (s_stackCurSubroutineBodyZone.size() == 0)  {
		return nullptr;
	}
	SubroutineBodyNode* node = (SubroutineBodyNode*)s_stackCurSubroutineBodyZone.top();
	return node;
}

void GramTreeNodeBase::insertSubRoutineBodyNode(SubroutineBodyNode* node)
{
	s_stackCurSubroutineBodyZone.push(node);
}

void GramTreeNodeBase::quitSubRoutineBodyZone()  
{
	assert(s_stackCurSubroutineBodyZone.size() > 0);
	s_stackCurSubroutineBodyZone.pop();
}




CompondStatement* GramTreeNodeBase::getCurCompoundStatmentNode()  
{
	if (s_stackCurCompoundStatmentZone.size() == 0)  {
		return nullptr;
	}
	return s_stackCurCompoundStatmentZone.top();
}

void GramTreeNodeBase::insertCompoundStatmentNode(CompondStatement* node)  
{    //在if while 开始后进入
	s_stackCurCompoundStatmentZone.push(node);
}

void GramTreeNodeBase::quitCompoundStatmentZone()  
{
	assert(s_stackCurCompoundStatmentZone.size() > 0);
	s_stackCurCompoundStatmentZone.pop();
}




string ClassTreeNode::getName()
{
	auto pNode = _child[ClassFiled::Class_Name];
	return pNode->getLexeme();
}



string SubroutineDecNode::getName()
{
	auto pNode = _child[SubroutineFiled::Name];
	return pNode->getLexeme();
}

string SubroutineDecNode::getSignName()
{
	auto pNode = _child[SubroutineFiled::Sign];
	return pNode->getLexeme();
}

int SubroutineDecNode::getFuncLocalsNum()
{
	return getSubroutineBody()->getFuncLocalsNum();
}

SubroutineBodyNode* SubroutineDecNode::getSubroutineBody()
{
	return (SubroutineBodyNode*)_child[SubroutineFiled::Body];
}

string SubroutineDecNode::getRetType()
{
	return getChildByIndex(SubroutineFiled::Ret)->getLexeme();
} 

GramTreeNodeBase* SubroutineDecNode::getFirstParam()
{
	return _child[SubroutineFiled::Params];
}

bool SubroutineDecNode::hasVarDecInParams(GramTreeNodeBase* node)
{
	auto params = _child[SubroutineDecNode::Params];
	while (params)  {
		if (params->getChildByIndex(VarDecNode::VarDec_Name)->getLexeme() == node->getLexeme())  {
			return true;
		}
		params = params->getNextNode();
	}
	return false;
}






int SubroutineBodyNode::getFuncLocalsNum()
{
	if (!getChildByIndex(SubroutineBody::VarDec))  {   //没有变量声明
		return 0;
	}
	int nlocals = 0;
	/*for (auto q = getChildByIndex(SubroutineBody::VarDec); q != nullptr; q = q->getNextNode())  {
		for (auto p = q->getChildByIndex(VarDecNode::EVarDec::VarDec_Name); p != nullptr; p = p->getNextNode())  {
			nlocals++;
		}
	}*/
	for (auto q = getChildByIndex(SubroutineBody::VarDec); q != nullptr; q = q->getNextNode())  {
		nlocals = nlocals + q->getChildByIndex(VarDecNode::EVarDec::VarDec_Name)->getSiblings();
	}
	return nlocals;
}

bool SubroutineBodyNode::hasVarDec(GramTreeNodeBase* node)  {
	auto p = _varDecList.getHeadNode();
	if (!p)  {
		return true;
	}
	while (p)  {
		auto var_name = p->getChildByIndex(VarDecNode::VarDec_Name);
		for (; var_name != nullptr; var_name = var_name->getNextNode())  {
			if (var_name->getLexeme() == node->getLexeme())  {
				return true;
			}
		}
		p = p->getNextNode();
	}
	return false;
}

VarDecNode* SubroutineBodyNode::getCurVarDec()  {
	return (VarDecNode*)_varDecList.getCurNode();
}



GramTreeNodeBase* AssignStatement::getChildByTag(string name)
{
	if (name == "var_name")  {
		return _child[AssignLetf];
	}
	else if (name == "var_rval")  {
		return _child[AssignRight];
	}
	return nullptr;
}

GramTreeNodeBase* AssignStatement::getAssginLeft()
{
	return _child[AssignLetf];
}

GramTreeNodeBase* AssignStatement::getAssginRight()
{
	return _child[AssignRight];
}




GramTreeNodeBase* VarDecNode::getVarDecType()
{
	return _child[VarDecNode::VarDec_Type];
}

GramTreeNodeBase* VarDecNode::getVarDecName()
{
	return _child[VarDecNode::VarDec_Name];
}