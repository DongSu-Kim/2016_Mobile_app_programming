#include "Assemble.h"
#include "AssembleMenu.h"
#include "GameManager.h"
#include "SoundManager.h"

CAssemble::CAssemble()
{
	p_cur_component_ = NULL;
}


CAssemble::~CAssemble() 
{
}

bool CAssemble::init()
{
	Layer::init();
	initTouch();
	
	//bgpng
	auto p_bg = Sprite::create("assemble/bg.png");
	p_bg->setPosition(Director::getInstance()->getWinSize() / 2);
	addChild(p_bg);

	//desk
	p_desk_ = CDesk::create();
	addChild(p_desk_);
	 
	p_assemble_menu_ = CAssembleMenu::create();
	addChild(p_assemble_menu_);

	//����ó�� ���� ���� �� Ȱ��ȭ
	p_assemble_menu_->rightMenuDisable("cpu");
	p_assemble_menu_->rightMenuDisable("ram");
	p_assemble_menu_->rightMenuDisable("power");
	p_assemble_menu_->rightMenuDisable("vga");
	p_assemble_menu_->rightMenuDisable("storage");


	Menu* menu = Menu::create();
	menu->setPosition(0, 0);
	addChild(menu);

	p_complete_ = MenuItemImage::create("assemble/complete_1.png", "assemble/complete_2.png", "assemble/complete_3.png", [=](...){
		//���� ���
		int false_count = 0;

		CGameManager::getInstance()->b_is_result_ = true;

		if (abs(p_desk_->getScore(component_type_cpu) - CGameManager::getInstance()->n_cpu_score_) > 5)
			false_count++;

		if (abs(p_desk_->getScore(component_type_ram) - CGameManager::getInstance()->n_ram_score_) > 5)
			false_count++;

		if (abs(p_desk_->getScore(component_type_power) - CGameManager::getInstance()->n_power_score_) > 5)
			false_count++;

		if (abs(p_desk_->getScore(component_type_vga) - CGameManager::getInstance()->n_vga_score_) > 5)
			false_count++;

		if (abs(p_desk_->getScore(component_type_storage) - CGameManager::getInstance()->n_storage_score_) > 5)
			false_count++;

		if (false_count >= 2)
			CGameManager::getInstance()->b_is_success_ = false;
		else
			CGameManager::getInstance()->b_is_success_ = true;

		CGameManager::getInstance()->changeState(e_state_room);
	});
	p_complete_->setPosition(905, 720 - p_complete_->getContentSize().height / 4);
	p_complete_->setEnabled(false);
	menu->addChild(p_complete_);


	p_explain_ = CExplain::create();
	addChild(p_explain_);

	auto price_tag = Sprite::create("assemble/price_tag.png");
	price_tag->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
	price_tag->setPosition(589, 720 - 658);
	addChild(price_tag);

	p_price_label_ = Label::create("0", "BM-HANNA.ttf", 40);
	p_price_label_->setPosition(589 + 118, 720 - 653);
	p_price_label_->setTextColor(Color4B::BLACK);
	p_price_label_->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
	addChild(p_price_label_);


	return true;
}


void CAssemble::initTouch()
{
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(CAssemble::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(CAssemble::onTouchMoved, this); //[=](Touch* touch, Event* _event){};
	listener->onTouchEnded = CC_CALLBACK_2(CAssemble::onTouchEnded, this); //[=](Touch*, Event*){};
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}
bool CAssemble::onTouchBegan(Touch* touch, Event* unused_event)
{
	p_assemble_menu_->touchBegan(touch);

	p_desk_->checkDeleteComp(touch->getLocation());
	if (p_desk_->p_cur_delete_comp_)
	{
		p_cur_component_ = p_desk_->p_cur_delete_comp_;
		p_desk_->p_cur_delete_comp_ = NULL;

		p_explain_->initWithData(p_cur_component_->getTypeToName(), p_cur_component_->n_idx_);
	}
	else
	{
		if (p_assemble_menu_->str_selected_name_ != "" && p_assemble_menu_->n_selected_idx_ != -1 )
		{
			p_explain_->initWithData(p_assemble_menu_->str_selected_name_, p_assemble_menu_->n_selected_idx_);
		}
		schedule([=](...){
			if (p_assemble_menu_->b_is_selected_)
			{
				p_cur_component_ = CComponentFactory::createComponent(p_assemble_menu_->str_selected_name_, p_assemble_menu_->n_selected_value_, p_assemble_menu_->n_selected_idx_);
				p_cur_component_->setPosition(touch->getLocation());
				addChild(p_cur_component_);
				p_desk_->ready(p_cur_component_);
				unschedule("select_check");
			}

		}, "select_check");
	}
	p_explain_->touchBegan(touch);
	return true;
}
void CAssemble::onTouchMoved(Touch* touch, Event* _event)
{	
	if (p_cur_component_)
	{
		p_cur_component_->setPosition(touch->getLocation());
		p_desk_->compMove(p_cur_component_, touch->getLocation());
	}
	else
		p_assemble_menu_->touchMove(touch);

	p_explain_->touchMove(touch);
}
void CAssemble::onTouchEnded(Touch* touch, Event* _event)
{
	p_assemble_menu_->touchEnd(touch);

	if (p_cur_component_){
		unschedule("select_check");
		if (p_desk_->attach(p_cur_component_))
		{

			if (!p_desk_->getLeftPosition(p_cur_component_))	//���� �ڸ��� �ϳ��� ���ٸ�,
			{
				//���� �޴� �Ұ��ϰ� ����
				p_assemble_menu_->rightMenuDisable(p_cur_component_->getTypeToName());
				p_assemble_menu_->p_left_menu_->clear();
				p_assemble_menu_->p_cur_touched_node_ = NULL;
			}
			
			if (p_cur_component_->getComponentType() == component_type_mainboard && p_desk_->vec_component_.size() == 1)
			{
				p_assemble_menu_->rightMenuEnable("cpu");
				p_assemble_menu_->rightMenuEnable("power");
				p_assemble_menu_->rightMenuEnable("ram");
				p_assemble_menu_->rightMenuEnable("power");
				p_assemble_menu_->rightMenuEnable("vga");
				p_assemble_menu_->rightMenuEnable("storage");
			}
			checkIsEnd();
		}
		else
		{
			//�������...

			//���� �޴� �����ϰ� �ϱ�
	
			p_assemble_menu_->rightMenuEnable(p_cur_component_->getTypeToName());
	
			if (p_cur_component_->getComponentType() == component_type_mainboard)
			{
				p_assemble_menu_->rightMenuDisable("cpu");
				p_assemble_menu_->rightMenuDisable("power");
				p_assemble_menu_->rightMenuDisable("ram");
				p_assemble_menu_->rightMenuDisable("power");
				p_assemble_menu_->rightMenuDisable("vga");
				p_assemble_menu_->rightMenuDisable("storage");
				p_assemble_menu_->p_left_menu_->clear();
			}

			p_cur_component_->removeFromParent();

			CSoundManager::getInstance()->playSfx("sound/detach.mp3");

			checkIsEnd();
		}
	}
	p_cur_component_ = NULL;


	//���� �����
	char str[64];
	util_add_comma_to_num(StringUtils::toString(p_desk_->getTotalPrice()).c_str(), str, 64);
	p_price_label_->setString(StringUtils::toString(str) + ANSIToUTF8("��"));

	if (p_desk_->getTotalPrice() > CGameManager::getInstance()->n_money_)
		p_price_label_->setTextColor(Color4B::RED);
	else
		p_price_label_->setTextColor(Color4B::BLACK);
}

void CAssemble::checkIsEnd()
{
	if (p_desk_->checkIsEnd() && p_desk_->getTotalPrice() <= CGameManager::getInstance()->n_money_)
	{
		p_complete_->setEnabled(true);
	}
	else
		p_complete_->setEnabled(false);
}