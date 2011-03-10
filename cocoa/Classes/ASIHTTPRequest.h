//
//  ASIHTTPRequest.h
//
//  Created by Ben Copsey on 04/10/2007.
//  Copyright 2007-2009 All-Seeing Interactive. All rights reserved.
//
//  A guide to the main features is available at:
//  http://allseeing-i.com/ASIHTTPRequest
//
//  Portions are based on the ImageClient example from Apple:
//  See: http://developer.apple.com/samplecode/ImageClient/listing37.html

#import <Foundation/Foundation.h>
#if TARGET_OS_IPHONE
	#import <CFNetwork/CFNetwork.h>
#endif
#import <stdio.h>

// Make targeting 2.2.1 more reliable
// See: http://www.blumtnwerx.com/blog/2009/06/cross-sdk-code-hygiene-in-xcode/
#ifndef __IPHONE_3_0
	#define __IPHONE_3_0 30000
#endif


typedef enum _ASINetworkErrorType {
    ASIConnectionFailureErrorType = 1,
    ASIRequestTimedOutErrorType = 2,
    ASIAuthenticationErrorType = 3,
    ASIRequestCancelledErrorType = 4,
    ASIUnableToCreateRequestErrorType = 5,
    ASIInternalErrorWhileBuildingRequestType  = 6,
    ASIInternalErrorWhileApplyingCredentialsType  = 7,
	ASIFileManagementError = 8,
	ASITooMuchRedirectionErrorType = 9,
	ASIUnhandledExceptionError = 10
	
} ASINetworkErrorType;

// The error domain that all errors generated by ASIHTTPRequest use
extern NSString* const NetworkRequestErrorDomain;

// You can use this number to throttle upload and download bandwidth in iPhone OS apps send or receive a large amount of data
// This may help apps that might otherwise be rejected for inclusion into the app store for using excessive bandwidth
// This number is not official, as far as I know there is no officially documented bandwidth limit
extern unsigned long const ASIWWANBandwidthThrottleAmount;

@interface ASIHTTPRequest : NSOperation {
	
	// The url for this operation, should include GET params in the query string where appropriate
	NSURL *url; 
	
	// The delegate, you need to manage setting and talking to your delegate in your subclasses
	id delegate;
	
	// A queue delegate that should *ALSO* be notified of delegate message (used by ASINetworkQueue)
	id queue;
	
	// HTTP method to use (GET / POST / PUT / DELETE / HEAD). Defaults to GET
	NSString *requestMethod;
	
	// Request body - only used when the whole body is stored in memory (shouldStreamPostDataFromDisk is false)
	NSMutableData *postBody;
	
	// gzipped request body used when shouldCompressRequestBody is YES
	NSData *compressedPostBody;
	
	// When true, post body will be streamed from a file on disk, rather than loaded into memory at once (useful for large uploads)
	// Automatically set to true in ASIFormDataRequests when using setFile:forKey:
	BOOL shouldStreamPostDataFromDisk;
	
	// Path to file used to store post body (when shouldStreamPostDataFromDisk is true)
	// You can set this yourself - useful if you want to PUT a file from local disk 
	NSString *postBodyFilePath;
	
	// Path to a temporary file used to store a deflated post body (when shouldCompressPostBody is YES)
	NSString *compressedPostBodyFilePath;
	
	// Set to true when ASIHTTPRequest automatically created a temporary file containing the request body (when true, the file at postBodyFilePath will be deleted at the end of the request)
	BOOL didCreateTemporaryPostDataFile;
	
	// Used when writing to the post body when shouldStreamPostDataFromDisk is true (via appendPostData: or appendPostDataFromFile:)
	NSOutputStream *postBodyWriteStream;
	
	// Used for reading from the post body when sending the request
	NSInputStream *postBodyReadStream;
	
	// Dictionary for custom HTTP request headers
	NSMutableDictionary *requestHeaders;
	
	// Set to YES when the request header dictionary has been populated, used to prevent this happening more than once
	BOOL haveBuiltRequestHeaders;
	
	// Will be populated with HTTP response headers from the server
	NSDictionary *responseHeaders;
	
	// Can be used to manually insert cookie headers to a request, but it's more likely that sessionCookies will do this for you
	NSMutableArray *requestCookies;
	
	// Will be populated with cookies
	NSArray *responseCookies;
	
	// If use useCookiePersistance is true, network requests will present valid cookies from previous requests
	BOOL useCookiePersistance;
	
	// If useKeychainPersistance is true, network requests will attempt to read credentials from the keychain, and will save them in the keychain when they are successfully presented
	BOOL useKeychainPersistance;
	
	// If useSessionPersistance is true, network requests will save credentials and reuse for the duration of the session (until clearSession is called)
	BOOL useSessionPersistance;
	
	// If allowCompressedResponse is true, requests will inform the server they can accept compressed data, and will automatically decompress gzipped responses. Default is true.
	BOOL allowCompressedResponse;
	
	// If shouldCompressRequestBody is true, the request body will be gzipped. Default is false.
	// You will probably need to enable this feature on your webserver to make this work. Tested with apache only.
	BOOL shouldCompressRequestBody;
	
	// When downloadDestinationPath is set, the result of this request will be downloaded to the file at this location
	// If downloadDestinationPath is not set, download data will be stored in memory
	NSString *downloadDestinationPath;
	
	//The location that files will be downloaded to. Once a download is complete, files will be decompressed (if necessary) and moved to downloadDestinationPath
	NSString *temporaryFileDownloadPath;
	
	// Used for writing data to a file when downloadDestinationPath is set
	NSOutputStream *fileDownloadOutputStream;
	
	// When the request fails or completes successfully, complete will be true
	BOOL complete;
	
	// If an error occurs, error will contain an NSError
	// If error code is = ASIConnectionFailureErrorType (1, Connection failure occurred) - inspect [[error userInfo] objectForKey:NSUnderlyingErrorKey] for more information
	NSError *error;
	
	// Username and password used for authentication
	NSString *username;
	NSString *password;
	
	// Domain used for NTLM authentication
	NSString *domain;
	
	// Username and password used for proxy authentication
	NSString *proxyUsername;
	NSString *proxyPassword;
	
	// Domain used for NTLM proxy authentication
	NSString *proxyDomain;
	
	// Delegate for displaying upload progress (usually an NSProgressIndicator, but you can supply a different object and handle this yourself)
	id uploadProgressDelegate;
	
	// Delegate for displaying download progress (usually an NSProgressIndicator, but you can supply a different object and handle this yourself)
	id downloadProgressDelegate;
	
	// Whether we've seen the headers of the response yet
    BOOL haveExaminedHeaders;
	
	// Data we receive will be stored here. Data may be compressed unless allowCompressedResponse is false - you should use [request responseData] instead in most cases
	NSMutableData *rawResponseData;
	
	// Used for sending and receiving data
    CFHTTPMessageRef request;	
	CFReadStreamRef readStream;
	
	// Used for authentication
    CFHTTPAuthenticationRef requestAuthentication; 
	NSMutableDictionary *requestCredentials;
	
	// Used during NTLM authentication
	int authenticationRetryCount;
	
	// Authentication scheme (Basic, Digest, NTLM)
	NSString *authenticationScheme;
	
	// Realm for authentication when credentials are required
	NSString *authenticationRealm;
	
	// And now, the same thing, but for authenticating proxies
	BOOL needsProxyAuthentication;
	
	// When YES, ASIHTTPRequest will present a dialog allowing users to enter credentials when no-matching credentials were found for a server that requires authentication
	// The dialog will not be shown if your delegate responds to authenticationNeededForRequest:
	// Default is NO.
	BOOL shouldPresentAuthenticationDialog;
	
	// When YES, ASIHTTPRequest will present a dialog allowing users to enter credentials when no-matching credentials were found for a proxy server that requires authentication
	// The dialog will not be shown if your delegate responds to proxyAuthenticationNeededForRequest:
	// Default is YES (basically, because most people won't want the hassle of adding support for authenticating proxies to their apps)
	BOOL shouldPresentProxyAuthenticationDialog;	
	
	// Used for proxy authentication
    CFHTTPAuthenticationRef proxyAuthentication; 
	NSMutableDictionary *proxyCredentials;
	
	// Used during authentication with an NTLM proxy
	int proxyAuthenticationRetryCount;
	
	// Authentication scheme for the proxy (Basic, Digest, NTLM)
	NSString *proxyAuthenticationScheme;	
	
	// Realm for proxy authentication when credentials are required
	NSString *proxyAuthenticationRealm;
	
	// HTTP status code, eg: 200 = OK, 404 = Not found etc
	int responseStatusCode;
	
	NSString *responseStatusMessage;
	
	// Size of the response
	unsigned long long contentLength;
	
	// Size of the partially downloaded content
	unsigned long long partialDownloadSize;
	
	// Size of the POST payload
	unsigned long long postLength;	
	
	// The total amount of downloaded data
	unsigned long long totalBytesRead;
	
	// The total amount of uploaded data
	unsigned long long totalBytesSent;
	
	// Last amount of data read (used for incrementing progress)
	unsigned long long lastBytesRead;
	
	// Last amount of data sent (used for incrementing progress)
	unsigned long long lastBytesSent;
	
	// This lock will block the request until the delegate supplies authentication info
	NSConditionLock *authenticationLock;
	
	// This lock prevents the operation from being cancelled at an inopportune moment
	NSRecursiveLock *cancelledLock;
	
	// Called on the delegate when the request starts
	SEL didStartSelector;
	
	// Called on the delegate when the request completes successfully
	SEL didFinishSelector;
	
	// Called on the delegate when the request fails
	SEL didFailSelector;
	
	// Used for recording when something last happened during the request, we will compare this value with the current date to time out requests when appropriate
	NSDate *lastActivityTime;
	
	// Number of seconds to wait before timing out - default is 10
	NSTimeInterval timeOutSeconds;
	
	// Will be YES when a HEAD request will handle the content-length before this request starts
	BOOL shouldResetProgressIndicators;
	
	// Used by HEAD requests when showAccurateProgress is YES to preset the content-length for this request
	ASIHTTPRequest *mainRequest;
	
	// When NO, this request will only update the progress indicator when it completes
	// When YES, this request will update the progress indicator according to how much data it has received so far
	// The default for requests is YES
	// Also see the comments in ASINetworkQueue.h
	BOOL showAccurateProgress;
	
	// Used to ensure the progress indicator is only incremented once when showAccurateProgress = NO
	BOOL updatedProgress;
	
	// Prevents the body of the post being built more than once (largely for subclasses)
	BOOL haveBuiltPostBody;
	
	// Used internally, may reflect the size of the internal buffer used by CFNetwork
	// POST / PUT operations with body sizes greater than uploadBufferSize will not timeout unless more than uploadBufferSize bytes have been sent
	// Likely to be 32KB on iPhone 3.0, 128KB on Mac OS X Leopard and iPhone 2.2.x
	unsigned long long uploadBufferSize;
	
	// Text encoding for responses that do not send a Content-Type with a charset value. Defaults to NSISOLatin1StringEncoding
	NSStringEncoding defaultResponseEncoding;
	
	// The text encoding of the response, will be defaultResponseEncoding if the server didn't specify. Can't be set.
	NSStringEncoding responseEncoding;
	
	// Tells ASIHTTPRequest not to delete partial downloads, and allows it to use an existing file to resume a download. Defaults to NO.
	BOOL allowResumeForFileDownloads;
	
	// Custom user information associated with the request
	NSDictionary *userInfo;
	
	// Use HTTP 1.0 rather than 1.1 (defaults to false)
	BOOL useHTTPVersionOne;
	
	// When YES, requests will automatically redirect when they get a HTTP 30x header (defaults to YES)
	BOOL shouldRedirect;
	
	// Used internally to tell the main loop we need to stop and retry with a new url
	BOOL needsRedirect;
	
	// Incremented every time this request redirects. When it reaches 5, we give up
	int redirectCount;
	
	// When NO, requests will not check the secure certificate is valid (use for self-signed cerficates during development, DO NOT USE IN PRODUCTION) Default is YES
	BOOL validatesSecureCertificate;
	
	// Details on the proxy to use - you could set these yourself, but it's probably best to let ASIHTTPRequest detect the system proxy settings
	NSString *proxyHost;
	int proxyPort;
	
	// URL for a PAC (Proxy Auto Configuration) file. If you want to set this yourself, it's probably best if you use a local file
	NSURL *PACurl;
	
	// True when request is attempting to handle an authentication challenge
	BOOL authenticationChallengeInProgress;
	
	// When YES, ASIHTTPRequests will present credentials from the session store for requests to the same server before being asked for them
	// This avoids an extra round trip for requests after authentication has succeeded, which is much for efficient for authenticated requests with large bodies, or on slower connections
	// Set to NO to only present credentials when explictly asked for them
	// This only affects credentials stored in the session cache when useSessionPersistance is YES. Credentials from the keychain are never presented unless the server asks for them
	// Default is YES
	BOOL shouldPresentCredentialsBeforeChallenge;
}

#pragma mark init / dealloc

// Should be an HTTP or HTTPS url, may include username and password if appropriate
- (id)initWithURL:(NSURL *)newURL;

// Convenience constructor
+ (id)requestWithURL:(NSURL *)newURL;

#pragma mark setup request

// Add a custom header to the request
- (void)addRequestHeader:(NSString *)header value:(NSString *)value;

// Populate the request headers dictionary. Called before a request is started, or by a HEAD request that needs to borrow them
- (void)buildRequestHeaders;

// Used to apply authorization header to a request before it is sent (when shouldPresentCredentialsBeforeChallenge is YES)
- (void)applyAuthorizationHeader;

// Create the post body
- (void)buildPostBody;

// Called to add data to the post body. Will append to postBody when shouldStreamPostDataFromDisk is false, or write to postBodyWriteStream when true
- (void)appendPostData:(NSData *)data;
- (void)appendPostDataFromFile:(NSString *)file;

#pragma mark get information about this request

// Returns the contents of the result as an NSString (not appropriate for binary data - used responseData instead)
- (NSString *)responseString;

// Response data, automatically uncompressed where appropriate
- (NSData *)responseData;

// Returns true if the response was gzip compressed
- (BOOL)isResponseCompressed;

#pragma mark running a request

// Run a request asynchronously by adding it to the global queue
// (Use [request start] for a synchronous request)
- (void)startAsynchronous;

#pragma mark request logic

// Main request loop is in here
- (void)loadRequest;

// Start the read stream. Called by loadRequest, and again to restart the request when authentication is needed
- (void)startRequest;

// Call to delete the temporary file used during a file download (if it exists)
// No need to call this if the request succeeds - it is removed automatically
- (void)removeTemporaryDownloadFile;

// Call to remove the file used as the request body
// No need to call this if the request succeeds and you didn't specify postBodyFilePath manually - it is removed automatically
- (void)removePostDataFile;

#pragma mark HEAD request

// Used by ASINetworkQueue to create a HEAD request appropriate for this request with the same headers (though you can use it yourself)
- (ASIHTTPRequest *)HEADRequest;

#pragma mark upload/download progress

- (void)updateProgressIndicators;
- (void)resetUploadProgress:(unsigned long long)value;
- (void)updateUploadProgress;
- (void)resetDownloadProgress:(unsigned long long)value;
- (void)updateDownloadProgress;

// Called when authorisation is needed, as we only find out we don't have permission to something when the upload is complete
- (void)removeUploadProgressSoFar;

// Helper method for interacting with progress indicators to abstract the details of different APIS (NSProgressIndicator and UIProgressView)
+ (void)setProgress:(double)progress forProgressIndicator:(id)indicator;

#pragma mark handling request complete / failure

// Called when a request starts, lets the delegate now via didStartSelector
- (void)requestStarted;

// Called when a request completes successfully, lets the delegate now via didFinishSelector
- (void)requestFinished;

// Called when a request fails, and lets the delegate now via didFailSelector
- (void)failWithError:(NSError *)theError;

#pragma mark parsing HTTP response headers

// Reads the response headers to find the content length, encoding, cookies for the session 
// Also initiates request redirection when shouldRedirect is true
// Returns true if the request needs a username and password (or if those supplied were incorrect)
- (BOOL)readResponseHeadersReturningAuthenticationFailure;

#pragma mark http authentication stuff

// Apply credentials to this request
- (BOOL)applyCredentials:(NSDictionary *)newCredentials;
- (BOOL)applyProxyCredentials:(NSDictionary *)newCredentials;

// Attempt to obtain credentials for this request from the URL, username and password or keychain
- (NSMutableDictionary *)findCredentials;
- (NSMutableDictionary *)findProxyCredentials;

// Unlock (unpause) the request thread so it can resume the request
// Should be called by delegates when they have populated the authentication information after an authentication challenge
- (void)retryUsingSuppliedCredentials;

// Should be called by delegates when they wish to cancel authentication and stop
- (void)cancelAuthentication;

// Apply authentication information and resume the request after an authentication challenge
- (void)attemptToApplyCredentialsAndResume;
- (void)attemptToApplyProxyCredentialsAndResume;

// Attempt to show the built-in authentication dialog, returns YES if credentials were supplied, NO if user cancelled dialog / dialog is disabled / running on main thread
// Currently only used on iPhone OS
- (BOOL)showProxyAuthenticationDialog;
- (BOOL)showAuthenticationDialog;

// Construct a basic authentication header from the username and password supplied, and add it to the request headers
// Used when shouldPresentCredentialsBeforeChallenge is YES
- (void)addBasicAuthenticationHeaderWithUsername:(NSString *)theUsername andPassword:(NSString *)thePassword;

#pragma mark stream status handlers

// CFnetwork event handlers
- (void)handleNetworkEvent:(CFStreamEventType)type;
- (void)handleBytesAvailable;
- (void)handleStreamComplete;
- (void)handleStreamError;

#pragma mark global queue

+ (NSOperationQueue *)sharedRequestQueue;

#pragma mark session credentials

+ (NSMutableArray *)sessionProxyCredentialsStore;
+ (NSMutableArray *)sessionCredentialsStore;

+ (void)storeProxyAuthenticationCredentialsInSessionStore:(NSDictionary *)credentials;
+ (void)storeAuthenticationCredentialsInSessionStore:(NSDictionary *)credentials;

+ (void)removeProxyAuthenticationCredentialsFromSessionStore:(NSDictionary *)credentials;
+ (void)removeAuthenticationCredentialsFromSessionStore:(NSDictionary *)credentials;

- (NSDictionary *)findSessionProxyAuthenticationCredentials;
- (NSDictionary *)findSessionAuthenticationCredentials;


#pragma mark keychain storage

// Save credentials for this request to the keychain
- (void)saveCredentialsToKeychain:(NSDictionary *)newCredentials;

// Save credentials to the keychain
+ (void)saveCredentials:(NSURLCredential *)credentials forHost:(NSString *)host port:(int)port protocol:(NSString *)protocol realm:(NSString *)realm;
+ (void)saveCredentials:(NSURLCredential *)credentials forProxy:(NSString *)host port:(int)port realm:(NSString *)realm;

// Return credentials from the keychain
+ (NSURLCredential *)savedCredentialsForHost:(NSString *)host port:(int)port protocol:(NSString *)protocol realm:(NSString *)realm;
+ (NSURLCredential *)savedCredentialsForProxy:(NSString *)host port:(int)port protocol:(NSString *)protocol realm:(NSString *)realm;

// Remove credentials from the keychain
+ (void)removeCredentialsForHost:(NSString *)host port:(int)port protocol:(NSString *)protocol realm:(NSString *)realm;
+ (void)removeCredentialsForProxy:(NSString *)host port:(int)port realm:(NSString *)realm;

// We keep track of any cookies we accept, so that we can remove them from the persistent store later
+ (void)setSessionCookies:(NSMutableArray *)newSessionCookies;
+ (NSMutableArray *)sessionCookies;

// Adds a cookie to our list of cookies we've accepted, checking first for an old version of the same cookie and removing that
+ (void)addSessionCookie:(NSHTTPCookie *)newCookie;

// Dump all session data (authentication and cookies)
+ (void)clearSession;

#pragma mark gzip decompression

// Uncompress gzipped data with zlib
+ (NSData *)uncompressZippedData:(NSData*)compressedData;

// Uncompress gzipped data from a file into another file, used when downloading to a file
+ (int)uncompressZippedDataFromFile:(NSString *)sourcePath toFile:(NSString *)destinationPath;
+ (int)uncompressZippedDataFromSource:(FILE *)source toDestination:(FILE *)dest;

#pragma mark gzip compression

// Compress data with gzip using zlib
+ (NSData *)compressData:(NSData*)uncompressedData;

// gzip compress data from a file, saving to another file, used for uploading when shouldCompressRequestBody is true
+ (int)compressDataFromFile:(NSString *)sourcePath toFile:(NSString *)destinationPath;
+ (int)compressDataFromSource:(FILE *)source toDestination:(FILE *)dest;

#pragma mark get user agent

// Will be used as a user agent if requests do not specify a custom user agent
// Is only used when you have specified a Bundle Display Name (CFDisplayBundleName) or Bundle Name (CFBundleName) in your plist
+ (NSString *)defaultUserAgentString;

#pragma mark proxy autoconfiguration

// Returns an array of proxies to use for a particular url, given the url of a PAC script
+ (NSArray *)proxiesForURL:(NSURL *)theURL fromPAC:(NSURL *)pacScriptURL;

#pragma mark mime-type detection

// Only works on Mac OS, will always return 'application/octet-stream' on iPhone
+ (NSString *)mimeTypeForFileAtPath:(NSString *)path;

#pragma mark bandwidth measurement / throttling

// The maximum number of bytes ALL requests can send / receive in a second
// This is a rough figure. The actual amount used will be slightly more, this does not include HTTP headers
+ (unsigned long)maxBandwidthPerSecond;
+ (void)setMaxBandwidthPerSecond:(unsigned long)bytes;

// Get a rough average (for the last 5 seconds) of how much bandwidth is being used, in bytes
+ (unsigned long)averageBandwidthUsedPerSecond;

// Will return YES is bandwidth throttling is currently in use
+ (BOOL)isBandwidthThrottled;

// Used internally to record bandwidth use, and by ASIInputStreams when uploading. It's probably best if you don't mess with this.
+ (void)incrementBandwidthUsedInLastSecond:(unsigned long)bytes;

// On iPhone, ASIHTTPRequest can automatically turn throttling on and off as the connection type changes between WWAN and WiFi

#if TARGET_OS_IPHONE
// Set to YES to automatically turn on throttling when WWAN is connected, and automatically turn it off when it isn't
+ (void)setShouldThrottleBandwidthForWWAN:(BOOL)throttle;

// Turns on throttling automatically when WWAN is connected using a custom limit, and turns it off automatically when it isn't
+ (void)throttleBandwidthForWWANUsingLimit:(unsigned long)limit;

// Called when the status of the network changes
+ (void)reachabilityChanged:(NSNotification *)note;
#endif

// Returns the maximum amount of data we can read as part of the current measurement period, and sleeps this thread if our allowance is used up
+ (unsigned long)maxUploadReadLength;

#pragma mark miscellany 

// Determines whether we're on iPhone OS 2.0 at runtime, currently used to determine whether we should apply a workaround for an issue with converting longs to doubles on iPhone OS 2
+ (BOOL)isiPhoneOS2;

// Used for generating Authorization header when using basic authentication when shouldPresentCredentialsBeforeChallenge is true
// And also by ASIS3Request
+ (NSString *)base64forData:(NSData *)theData;

#pragma mark ===

@property (retain) NSString *username;
@property (retain) NSString *password;
@property (retain) NSString *domain;

@property (retain) NSString *proxyUsername;
@property (retain) NSString *proxyPassword;
@property (retain) NSString *proxyDomain;

@property (retain) NSString *proxyHost;
@property (assign) int proxyPort;

@property (retain,setter=setURL:) NSURL *url;
@property (assign) id delegate;
@property (assign) id queue;
@property (assign) id uploadProgressDelegate;
@property (assign) id downloadProgressDelegate;
@property (assign) BOOL useKeychainPersistance;
@property (assign) BOOL useSessionPersistance;
@property (retain) NSString *downloadDestinationPath;
@property (retain) NSString *temporaryFileDownloadPath;
@property (assign) SEL didStartSelector;
@property (assign) SEL didFinishSelector;
@property (assign) SEL didFailSelector;
@property (retain,readonly) NSString *authenticationRealm;
@property (retain,readonly) NSString *proxyAuthenticationRealm;
@property (retain) NSError *error;
@property (assign,readonly) BOOL complete;
@property (retain,readonly) NSDictionary *responseHeaders;
@property (retain) NSMutableDictionary *requestHeaders;
@property (retain) NSMutableArray *requestCookies;
@property (retain,readonly) NSArray *responseCookies;
@property (assign) BOOL useCookiePersistance;
@property (retain) NSDictionary *requestCredentials;
@property (retain) NSDictionary *proxyCredentials;
@property (assign,readonly) int responseStatusCode;
@property (retain,readonly) NSString *responseStatusMessage;
@property (retain,readonly) NSMutableData *rawResponseData;
@property (assign) NSTimeInterval timeOutSeconds;
@property (retain) NSString *requestMethod;
@property (retain) NSMutableData *postBody;
@property (assign,readonly) unsigned long long contentLength;
@property (assign) unsigned long long postLength;
@property (assign) BOOL shouldResetProgressIndicators;
@property (retain) ASIHTTPRequest *mainRequest;
@property (assign) BOOL showAccurateProgress;
@property (assign,readonly) unsigned long long totalBytesRead;
@property (assign,readonly) unsigned long long totalBytesSent;
@property (assign) NSStringEncoding defaultResponseEncoding;
@property (assign,readonly) NSStringEncoding responseEncoding;
@property (assign) BOOL allowCompressedResponse;
@property (assign) BOOL allowResumeForFileDownloads;
@property (retain) NSDictionary *userInfo;
@property (retain) NSString *postBodyFilePath;
@property (assign) BOOL shouldStreamPostDataFromDisk;
@property (assign) BOOL didCreateTemporaryPostDataFile;
@property (assign) BOOL useHTTPVersionOne;
@property (assign, readonly) unsigned long long partialDownloadSize;
@property (assign) BOOL shouldRedirect;
@property (assign) BOOL validatesSecureCertificate;
@property (assign) BOOL shouldCompressRequestBody;
@property (assign) BOOL needsProxyAuthentication;
@property (retain) NSURL *PACurl;
@property (retain) NSString *authenticationScheme;
@property (retain) NSString *proxyAuthenticationScheme;
@property (assign) BOOL shouldPresentAuthenticationDialog;
@property (assign) BOOL shouldPresentProxyAuthenticationDialog;
@property (assign) BOOL authenticationChallengeInProgress;
@property (assign) BOOL shouldPresentCredentialsBeforeChallenge;
@property (assign, readonly) int authenticationRetryCount;
@property (assign, readonly) int proxyAuthenticationRetryCount;
@property (assign) BOOL haveBuiltRequestHeaders;
@property (assign, nonatomic) BOOL haveBuiltPostBody;
@end
