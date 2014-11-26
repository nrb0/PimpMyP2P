#include "products/common/messages/pimp_message.h"

PimpMessage::PimpMessage(juce::IPAddress srcIp)
: _message(new XmlElement("PimpMessage"))
{
  auto sourceIpXml = new XmlElement("SourceIp");
  sourceIpXml->setAttribute("Value", srcIp.toString());
  _message->addChildElement(sourceIpXml);
}

PimpMessage::PimpMessage(const std::string& xmlData)
{
  XmlDocument document (xmlData);
  _message = document.getDocumentElement();
  if (!_message)
  {
    _message = new XmlElement("PimpMessage");
    setCommand(kError);
  }
}

PimpMessage::~PimpMessage()
{
  _message = nullptr;
}

const juce::String PimpMessage::getXmlString() const
{
  return _message->createDocument(juce::String::empty);
}

const bool PimpMessage::isCommand(PimpMessage::CommandType cmd) const
{
  auto command = _message->getChildByName("Command");
  if (command)
    if (command->hasAttribute("Value"))
      return (CommandType)command->getIntAttribute("Value") == cmd;
  return false;
}

void PimpMessage::setCommand(PimpMessage::CommandType cmd)
{
  auto command = _message->getChildByName("Command");
  if (!command)
  {
    command = new XmlElement("Command");
    command->setAttribute("Value", cmd);
    _message->addChildElement(command);
  }
  else
  {
    command->setAttribute("Value", cmd);
  }
}

const juce::IPAddress PimpMessage::getSource() const
{
  auto sourceXml = _message->getChildByName("SourceIp");
  if (sourceXml)
    if (sourceXml->hasAttribute("Value"))
      return juce::IPAddress(sourceXml->getStringAttribute("Value"));
  return juce::IPAddress();
}

const bool PimpMessage::hasPeerFile() const
{
  auto file = _message->getChildByName("PeerFile");
  if (file)
    return file->hasAttribute("Name") &&
           file->hasAttribute("MD5") &&
           file->hasAttribute("Size");
  return false;
}

const PeerFile PimpMessage::getPeerFile() const
{
  auto file = _message->getChildByName("PeerFile");
  if (file)
  {
    auto nameXml = file->getChildByName("Name");
    auto sizeXml = file->getChildByName("Size");
    auto md5Xml = file->getChildByName("MD5");
    if (!nameXml || !sizeXml || !md5Xml)
      return PeerFile::emptyPeerFile();
    {
      return PeerFile(nameXml->getStringAttribute("Value"),
                      md5Xml->getStringAttribute("Value"),
                      sizeXml->getIntAttribute("Value"));
    }
  }
  return PeerFile::emptyPeerFile();
}

const bool PimpMessage::hasByteRange() const
{
  auto range = _message->getChildByName("Range");
  if (range)
    return range->hasAttribute("Start") && range->hasAttribute("End");
  return false;
}

const juce::Range<int> PimpMessage::getByteRange() const
{
  auto range = _message->getChildByName("Range");
  if (range)
    if (range->hasAttribute("Start") && range->hasAttribute("End"))
    {
      juce::Range<int> byteRange;
      byteRange.setStart(range->getIntAttribute("Start"));
      byteRange.setEnd(range->getIntAttribute("End"));
      return byteRange;
    }
  return juce::Range<int>::emptyRange(0);
}

const bool PimpMessage::isTrackerSearchResult() const
{
  return isCommand(kTrackerSearchResult);
}

const bool PimpMessage::hasSearchString() const
{
  auto searchStringXml = _message->getChildByName("SearchString");
  if (searchStringXml) return true;
  return false;
}

const juce::String PimpMessage::getSearchString() const
{
  auto searchStringXml = _message->getChildByName("SearchString");
  if (searchStringXml)
    return searchStringXml->getStringAttribute("Value");
  return juce::String::empty;
}

const bool PimpMessage::hasSearchResults() const
{
  auto searchResultsXml = _message->getChildByName("SearchResults");
  if (searchResultsXml) return true;
  return false;
}

const juce::Array<PeerFile> PimpMessage::getSearchResults() const
{
  juce::Array<PeerFile> result;
  auto searchResultsXml = _message->getChildByName("SearchResults");
  if (searchResultsXml)
  {
    for (int fileIndex = 0;
         fileIndex < searchResultsXml->getNumChildElements();
         fileIndex++)
    {
      auto file = searchResultsXml->getChildElement(fileIndex);
      if (file)
      {
        auto nameXml = file->getChildByName("Name");
        auto sizeXml = file->getChildByName("Size");
        auto md5Xml = file->getChildByName("MD5");
        auto peersXml = file->getChildByName("PeersList");
        if (nameXml && sizeXml && md5Xml && peersXml)
        {
          juce::Array<juce::IPAddress> peersList;
          for (int peerIndex = 0;
               peerIndex < peersXml->getNumChildElements();
               peerIndex++)
          {
            auto dummyPeer = peersXml->getChildElement(peerIndex);
            juce::IPAddress dummyIP (dummyPeer->getStringAttribute("Value"));
            peersList.add(juce::IPAddress(dummyIP));
          }
          result.add(PeerFile(nameXml->getStringAttribute("Value"),
                              md5Xml->getStringAttribute("Value"),
                              sizeXml->getIntAttribute("Value"),
                              peersList));
        }
      }
    }
  }
  return result;
}

const bool PimpMessage::isPeerRefresh() const
{
  return isCommand(kPeerRefresh);
}

const bool PimpMessage::isPeerSignOut() const
{
  return isCommand(kPeerSignOut);
}

void PimpMessage::sendToSocket(juce::StreamingSocket *socket)
{
  if (socket && socket->isConnected())
  {
    std::string out = getXmlString().toStdString();
    socket->write(out.c_str(), out.length());
  }
}

void PimpMessage::createPeerGetFile(PeerFile file)
{
  setCommand(kPeerGetFile);
  
  auto fileXml = new XmlElement("PeerFile");
  
  auto dummyName = new XmlElement("Name");
  dummyName->setAttribute("Value", file.getFilename());
  fileXml->addChildElement(dummyName);
  
  auto dummyMD5 = new XmlElement("MD5");
  dummyMD5->setAttribute("Value",file.getMD5());
  fileXml->addChildElement(dummyMD5);
  
  auto dummySize = new XmlElement("Size");
  dummySize->setAttribute("Value", (double)(file.getSize()));
  fileXml->addChildElement(dummySize);
  
  _message->addChildElement(fileXml);
}

void PimpMessage::createPeerGetFile(PeerFile file,
                                    juce::Range<int> byteRange)
{
  createPeerGetFile(file);
  auto rangeXml = new XmlElement("Range");
  
  auto dummyStart = new XmlElement("Start");
  dummyStart->setAttribute("Value", byteRange.getStart());
  rangeXml->addChildElement(dummyStart);
  
  auto dummyEnd = new XmlElement("End");
  dummyEnd->setAttribute("Value", byteRange.getEnd());
  rangeXml->addChildElement(dummyEnd);
  
  _message->addChildElement(rangeXml);
}

void PimpMessage::createPeerSearch(juce::String keystring)
{
  setCommand(kPeerSearch);
  
  auto searchStringXml = new XmlElement("SearchString");
  searchStringXml->setAttribute("Value", keystring);
  _message->addChildElement(searchStringXml);
}

void PimpMessage::createErrorMessage(juce::String message)
{
  setCommand(kError);
}

void PimpMessage::createTrackerSearchResult(const juce::Array<PeerFile>& files)
{
  setCommand(kTrackerSearchResult);
  
  auto resultsXml = new XmlElement("SearchResults");
  
  for (const PeerFile& file : files)
  {
    auto fileXml = new XmlElement("PeerFile");
    
    auto dummyName = new XmlElement("Name");
    dummyName->setAttribute("Value", file.getFilename());
    fileXml->addChildElement(dummyName);
    
    auto dummyMD5 = new XmlElement("MD5");
    dummyMD5->setAttribute("Value",file.getMD5());
    fileXml->addChildElement(dummyMD5);
    
    auto dummySize = new XmlElement("Size");
    dummySize->setAttribute("Value", (double)(file.getSize()));
    fileXml->addChildElement(dummySize);
    
    auto dummyPeersAddresses = new XmlElement("PeersList");
    for (const juce::IPAddress& address : file.getPeersAddresses())
    {
      auto dummyPeer = new XmlElement("Peer");
      dummyPeer->setAttribute("Value", address.toString());
      dummyPeersAddresses->addChildElement(dummyPeer);
    }
    fileXml->addChildElement(dummyPeersAddresses);
    
    resultsXml->addChildElement(fileXml);
  }
  
  _message->addChildElement(resultsXml);
}

void PimpMessage::createPeerRefresh(const juce::Array<PeerFile> localFiles)
{
  setCommand(kPeerRefresh);
  if (localFiles.size() != 0)
  {
    auto filelistXml = new XmlElement("LocalFileList");
    
    for (const PeerFile& file : localFiles)
    {
      auto fileXml = new XmlElement("PeerFile");
      
      auto dummyName = new XmlElement("Name");
      dummyName->setAttribute("Value", file.getFilename());
      fileXml->addChildElement(dummyName);
      
      auto dummyMD5 = new XmlElement("MD5");
      dummyMD5->setAttribute("Value",file.getMD5());
      fileXml->addChildElement(dummyMD5);
      
      auto dummySize = new XmlElement("Size");
      dummySize->setAttribute("Value", (double)(file.getSize()));
      fileXml->addChildElement(dummySize);
      
      filelistXml->addChildElement(fileXml);
    }
    
    _message->addChildElement(filelistXml);
  }
}

void PimpMessage::createPeerSignOut()
{
  setCommand(kPeerSignOut);
}

const bool PimpMessage::hasLocalFileList() const
{
  auto localFileListXml = _message->getChildByName("LocalFileList");
  if (localFileListXml) return true;
  return false;
}

const juce::Array<PeerFile> PimpMessage::getLocalFileList() const
{
  juce::Array<PeerFile> localFiles;
  auto localFilesXml = _message->getChildByName("LocalFileList");
  if (localFilesXml)
  {
    for (int fileIndex = 0;
         fileIndex < localFilesXml->getNumChildElements();
         fileIndex++)
    {
      auto file = localFilesXml->getChildElement(fileIndex);
      if (file)
      {
        auto nameXml = file->getChildByName("Name");
        auto sizeXml = file->getChildByName("Size");
        auto md5Xml = file->getChildByName("MD5");
        if (nameXml && sizeXml && md5Xml)
          localFiles.add(PeerFile(nameXml->getStringAttribute("Value"),
                                  md5Xml->getStringAttribute("Value"),
                                  sizeXml->getIntAttribute("Value")));
      }
    }
  }
  return localFiles;
}



