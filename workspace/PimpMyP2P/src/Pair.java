import java.io.File;
import java.net.Socket;
import java.security.interfaces.RSAPrivateKey;
import java.security.spec.RSAPublicKeySpec;
import java.util.ArrayList;


public class Pair extends Thread {
  private Socket _socket;
  private String _address;
  private File _downloadFolder;
  private String _trackerIp;
  private ArrayList<File> _availableFiles = new ArrayList<File>();
  private RSAPublicKeySpec _keyPublic;
  private RSAPrivateKey _keyPrivate;
  
  
  Pair()
  {
	  
  }
  
  public void run()
  {
	  
  }
  
  public void requestFileFromPeer(String peerIp, String targetFile)
  {
	  
  }
  
  public void requestFileFromPeer(ArrayList<String> peerIps, String targetFile)
  {
	  
  }
  
  public void requestPartFromPeer(String peerIp, String targetFile, int startIndex, int endIndex)
  {
	  
  }
  
  public void sendAvailableFilesListToTracker(String trackerIp)
  {
	  
  }
  
  
}
