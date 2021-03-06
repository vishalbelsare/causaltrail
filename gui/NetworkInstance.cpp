#include "NetworkInstance.h"
#include "DiscretisationSelection.h"
#include "../core/Parser.h"
#include "NodeGui.h"
NetworkInstance::NetworkInstance(QWidget* parent)
    : QWidget(parent),
      nv_(nullptr),
      trained_(false),
      argmax_(false),
      remainingNodesForEdgeAddition_(0),
      id1_(-1),
      id2_(-1)
{
	discretisationSelection_ = new DiscretisationSelection(this);
	connect(discretisationSelection_, SIGNAL(accepted()), this, SLOT(loadSamples()));
}

void NetworkInstance::loadNetwork(QString filename){
    nc_.loadNetwork(filename.toStdString());
}

void NetworkInstance::loadNetwork() {
	nc_.loadNetwork(naOrTgf_.toStdString());
	if(sif_ != "") {
		nc_.loadNetwork(sif_.toStdString());
	}
}

void NetworkInstance::visualize() {
	delete nv_;

    nv_ = new NetworkVis(this, nc_.getNetwork());

	connect(nv_,  SIGNAL(context(Edge*, QContextMenuEvent*)),
	        this, SIGNAL(context(Edge*, QContextMenuEvent*)));
	connect(nv_,  SIGNAL(context(NodeGui*, QContextMenuEvent*)),
	        this, SIGNAL(context(NodeGui*, QContextMenuEvent*)));
	connect(nv_, SIGNAL(doubleClick(NodeGui*)), this, SIGNAL(doubleClick(NodeGui*)));
}

void NetworkInstance::loadSamples()
{
	loadSamples(discretisationSelection_->samples(), discretisationSelection_->getPropertyTree());
}

void NetworkInstance::loadSamples(const QString& samples, const QString& controlFile)
{
	loadSamples(samples, DiscretisationSettings(controlFile.toStdString()));
}

void NetworkInstance::loadSamples(const QString& samples, const DiscretisationSettings& settings)
{
	dataFile_ = samples;
	discretisationSettings_ = settings;

	try {
		nc_.loadObservations(dataFile_.toStdString(), settings, deselectedSamples_);
	} catch(const boost::property_tree::ptree_bad_data&) {
		emit newLogMessage("Error in discretisation control. Could not convert parameter to required type.");
		return;
	} catch(const boost::property_tree::ptree_bad_path&) {
		emit newLogMessage("Error in discretisation control. Required parameter not present.");
		return;
	} catch(const std::invalid_argument& e) {
		emit newLogMessage(QString("Error while loading samples: ") + e.what());
		return;
	}

	try {
		nc_.trainNetwork();
		trained_ = true;
		emit samplesLoaded(this);
	} catch(const std::invalid_argument& e) {
		emit newLogMessage(QString("Error while training the network: ") + e.what());
	}
}

std::pair<float, std::vector<std::string>>
NetworkInstance::calculate(const std::string& query)
{
	return Parser(query, nc_).parseQuery().execute();
}

bool NetworkInstance::isTrained(){
    return trained_;
}

unsigned int NetworkInstance::getRowCountOfCurrentProbMatrix()
{
    return nc_.getNetwork().getNode(selectedNode_).getProbabilityMatrix().getRowCount();
}

unsigned int NetworkInstance::getColCountOfCurrentProbMatrix()
{
    return nc_.getNetwork().getNode(selectedNode_).getProbabilityMatrix().getColCount();
}

unsigned int NetworkInstance::getNumberOfParentsOfSelectedNode(){
    return nc_.getNetwork().getNode(selectedNode_).getNumberOfParents();
}

std::vector<std::string> NetworkInstance::getParentsOfSelectedNode(){
    auto parentList = nc_.getNetwork().getNode(selectedNode_).getParents();
    std::vector<std::string> temp;
    for (auto& parent : parentList){
        temp.push_back(nc_.getNetwork().getNode(parent).getName());
    }
    return temp;
}

float NetworkInstance::getProbabilityAt(unsigned int row, unsigned int col)
{
    return nc_.getNetwork().getNode(selectedNode_).getProbabilityMatrix()(col,row);
}

const std::vector<std::string> &NetworkInstance::getRowNamesOfProbabilityMatrix()
{
   return nc_.getNetwork().getNode(selectedNode_).getProbabilityMatrix().getRowNames();
}

const std::vector<std::string> &NetworkInstance::getColNamesOfProbabilityMatrix()
{
   return nc_.getNetwork().getNode(selectedNode_).getProbabilityMatrix().getColNames();
}

void NetworkInstance::visualizeNonInterventionNodes(std::vector<QString> &nonInterventions)
{
    for (QString& item : nonInterventions){
        QString name = item.split(" ")[0];
        if (name == "argmax"){
            name = item.split(" ")[1];
        }
        QColor queryColor;
        queryColor.setHsv(100,30,250);
        nv_->getNode(nc_.getNetwork().getNode(name.toStdString()).getID())->originalState();
        nv_->getNode(nc_.getNetwork().getNode(name.toStdString()).getID())->setColor(queryColor);
    }
}

void NetworkInstance::visualizeCondition(std::vector<QString> &condition)
{
    for (QString& item : condition){
        QString name = item.split(" ")[0];
        QColor queryColor;
        queryColor.setHsv(20,30,250);
        nv_->getNode(nc_.getNetwork().getNode(name.toStdString()).getID())->originalState();
        nv_->getNode(nc_.getNetwork().getNode(name.toStdString()).getID())->setColor(queryColor);
    }
}

void NetworkInstance::visualizeInterventions(std::vector<QString> &interventions)
{
    for (QString& item : interventions){
        QString name = item.split(" ")[0];
        nv_->getNode(nc_.getNetwork().getNode(name.toStdString()).getID())->originalState();
        setSelectedNode(nc_.getNetwork().getNode(name.toStdString()).getID());
        doIntervention();
    }
}

void NetworkInstance::visualizeEdgeChanges(std::vector<QString> &edgeChanges)
{
   for (QString& item : edgeChanges){
        std::string id_ = item.split(" ")[0].toStdString();
        std::string name1 = item.split(" ")[1].toStdString();
        std::string name2 = item.split(" ")[2].toStdString();
        if (id_=="+"){
            nv_->addEdge(nc_.getNetwork().getNode(name1).getID(),nc_.getNetwork().getNode(name2).getID());
        }
        else{
            nv_->removeEdge(nc_.getNetwork().getNode(name1).getID(),nc_.getNetwork().getNode(name2).getID());
        }
    }
}

void NetworkInstance::layout(){
    nv_->layoutGraph();
}

const std::vector<std::string>& NetworkInstance::getValues(unsigned int id){
  return nc_.getNetwork().getNode(id).getValueNamesProb();
}

void NetworkInstance::setArgMax(bool flag){
    argmax_=flag;
}

bool NetworkInstance::isArgMax(){
    return argmax_;
}

void NetworkInstance::initialiseEdgeAddition(){
    remainingNodesForEdgeAddition_=2;
    id1_=-1;
    id2_=-1;
}

bool NetworkInstance::edgeAddition(){
    return (remainingNodesForEdgeAddition_ > 0);
}

bool NetworkInstance::checkEdgeAddition(unsigned int id){
    std::vector<std::pair<unsigned int, unsigned int>> addedEdgeList = nv_->getAddedEdgeIDs();
    std::vector<std::pair<unsigned int, unsigned int>> removedEdgeList = nv_->getRemovedEdgeIDs();
    if (id1_ != -1){
        return nc_.isEdgePossible(id1_, id, addedEdgeList, removedEdgeList);
    }
    return false;
}

void NetworkInstance::nodeForEdgeAdditionSelected(unsigned int id){
    if (id1_==-1){
        id1_=id;
    }
    else{
        id2_=id;
        nv_->addEdge(id1_,id2_);
        emit edgeAdded(id1_, id2_);
        id1_=-1;
        id2_=-1;
    }
    remainingNodesForEdgeAddition_--;
}

void NetworkInstance::addEdgeRemoval(unsigned int srcId, unsigned int tarId) {
   id1_ = srcId;
   id2_ = tarId;
}

const std::string& NetworkInstance::removedEdgeSourceName() const {
    return nc_.getNetwork().getNode(id1_).getName();
}

const std::string& NetworkInstance::removedEdgeTargetName() const {
    return nc_.getNetwork().getNode(id2_).getName();
}

void NetworkInstance::removeSelectedEdge(){
    nv_->removeEdge(id1_,id2_);
    emit edgeRemoved(id1_, id2_);
    id1_=-1;
    id2_=-1;
}

void NetworkInstance::setSelectedNode(unsigned int id)
{
    selectedNode_=id;
}

QString NetworkInstance::getSelectedNodeName() {
    return QString::fromStdString(nc_.getNetwork().getNode(selectedNode_).getName());
}

unsigned int NetworkInstance::getSelectedNodeID()
{
    return selectedNode_;
}

void NetworkInstance::doIntervention(){
    nv_->doIntervention(selectedNode_,true);
}

void NetworkInstance::removeDoIntervention(QString name){
    selectedNode_=nc_.getNetwork().getNode(name.toStdString()).getID();
    nv_->doIntervention(selectedNode_,false);
}

void NetworkInstance::removeHighlighting(){
	if(nv_) {
		nv_->removeNodeHighlighting();
		nv_->removeEdgeHighlighting();
	}
}

void NetworkInstance::restoreOriginalNetworkRepresentation(){
	if(nv_) {
		nv_->unDoDoInt();
		nv_->originalNodeState();
		nv_->restoreEdges();
		nv_->removeAdditionalEdges();
	}
}

void NetworkInstance::reverseEdgeRemoval(QString source, QString target){
    nv_->restoreEdge(source, target);
}

void NetworkInstance::reverseEdgeAddition(QString source, QString target){
    nv_->removeAdditionalEdge(source, target);
}

void NetworkInstance::colorNode(unsigned int id, QColor color){
    nv_->getNode(id)->setColor(color);
}

void NetworkInstance::removeNodeColor(QString name,QColor color){
    nv_->getNode(nc_.getNetwork().getNode(name.toStdString()).getID())->removeColor(color);
}

const QString& NetworkInstance::getNaOrTgf() const {
    return naOrTgf_;
}

const QString& NetworkInstance::getSif() const {
    return sif_;
}

const QString& NetworkInstance::getDataFile() const {
    return dataFile_;
}

void NetworkInstance::setNaOrTgf(const QString& filename){
    naOrTgf_= filename;
}

void NetworkInstance::setSif(const QString& filename){
    sif_= filename;
}

void NetworkInstance::setDataFile(const QString& filename){
    dataFile_= filename;
}

void NetworkInstance::setQMA(QueryManager& qma){
    qma_=qma;
}

const NetworkController& NetworkInstance::getController() const {
	return nc_;
}

NetworkController& NetworkInstance::getController() {
	return nc_;
}

QueryManager& NetworkInstance::getQMA() {
	return qma_;
}

const QueryManager& NetworkInstance::getQMA() const
{
	return qma_;
}

bool NetworkInstance::queryHistoryIsEmpty() const
{
    return (qma_.getNumberOfQueries()==0);
}

void NetworkInstance::resetNetwork(){
    nc_.getNetwork().reset();
}

void NetworkInstance::setDeselectedSamples(const std::vector<unsigned int> &ids)
{
    deselectedSamples_=ids;
}

const std::vector<unsigned int>& NetworkInstance::getDeselectedSamples() const
{
    return deselectedSamples_;
}

void NetworkInstance::exportSvg(const QString& filename)
{
	nv_->exportSVG(filename);
}

const DiscretisationSelection* NetworkInstance::getDiscretisationSelection() const
{
	return discretisationSelection_;	
}

const DiscretisationSettings& NetworkInstance::getDiscretisationSettings() const
{
	return discretisationSettings_;
}

void NetworkInstance::setDiscretisationSettings(const DiscretisationSettings& settings)
{
	discretisationSettings_ = settings;
}

void NetworkInstance::discretise(const QString& samples,
                                 const std::vector<uint>& deselected)
{
	setDeselectedSamples(deselected);
	discretisationSelection_->show(samples);
}

const std::string& NetworkInstance::getNodeName(unsigned int i) const {
	return getController().getNetwork().getNode(i).getName();
}

void NetworkInstance::saveDiscretisedData(const QString & filename) const{
	nc_.storeDiscretisedData(filename.toStdString());
}
