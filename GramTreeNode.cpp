#include "Scanner.h"
#include "GramTreeNode.h"

GramTreeNodeBase* GramTreeNodeBase::s_curVarDecType = nullptr;
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



SubroutineBodyNode* GramTreeNodeBase::getCurSubroutineBodyNode()             //目前在哪个函数体中,用于非复合语句添加树,可以用于闭包
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



bool GramTreeNodeBase::isInCompoundBody(GramTreeNodeBase* node)
{
	if (node && node->getParentNode())  {
		if (node->getParentNode()->getNodeKind() == GramTreeNodeBase::IF_STATEMENT_K ||
			node->getParentNode()->getNodeKind() == GramTreeNodeBase::WHILE_STATEMENT_K)  {
			if (node->getChildIndex() == 1)  {
				return true;
			}
		}
	}
	return false;
}

bool GramTreeNodeBase::isInCompound(GramTreeNodeBase* node)
{
	if (node)  {
		if (node->getNodeKind() == GramTreeNodeBase::IF_STATEMENT_K ||
			node->getNodeKind() == GramTreeNodeBase::WHILE_STATEMENT_K)  {
			return true;
		}
	}
	return false;
}






CompondStatement* GramTreeNodeBase::getCurCompoundStatmentNode(int* pNum)     //目前在哪个if 或while体中,用于添加if while 的语句
{
	if (s_stackCurCompoundStatmentZone.size() == 0)  {
		if (pNum)  { 
			*pNum = 0; 
		}
		return nullptr;
	}
	if (pNum)  { 
		*pNum = s_stackCurCompoundStatmentZone.size(); 
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
	if (!getChildByIndex(BaseBlockBody::VarDec))  {   //没有变量声明
		return 0;
	}
	int nlocals = 0;
	/*for (auto q = getChildByIndex(SubroutineBody::VarDec); q != nullptr; q = q->getNextNode())  {
		for (auto p = q->getChildByIndex(VarDecNode::EVarDec::VarDec_Name); p != nullptr; p = p->getNextNode())  {
			nlocals++;
		}
	}*/
	for (auto q = getChildByIndex(BaseBlockBody::VarDec); q != nullptr; q = q->getNextNode())  {
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











void TreeNodeList::Push(TreeNode* node)  
{     
	if (node != nullptr)  {
		if (_head == nullptr)  {
			TreeNode* curNode = getCurNode(node);
			if (curNode != node)  {  //要加入的节点是个链节点则要拆散一个一个的加
				_head = node;
				_cur = curNode;
			}
			else  {
				_head = _cur = node;
			}
		}
		else  {
			TreeNode* curNode = getCurNode(node);  //节点的当前节点,即最后一个节点
			if (curNode != node)  {                //要加入的节点是个链节点则要拆散一个一个的加
				_cur->setNextNode(node);
				_cur = curNode;
			}
			else  {
				_cur->setNextNode(node);
				_cur = node;
			}
		}
	}
}

TreeNode* TreeNodeList::getCurNode(TreeNode* node)  
{
	TreeNode* curNode = nullptr;
	while (node)  {
		curNode = node;
		node = node->getNextNode();
	}
	return curNode;
}

TreeNode* TreeNodeList::joinBy(TreeNodeList* node2)
{
	if (_cur)  {
		if (node2)  {
			_cur->setNextNode(node2->getHeadNode());
		}
	}
	else {
		if (node2)  {
			return node2->getHeadNode();
		}
	}
	return _head;
}