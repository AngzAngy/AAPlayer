package com.angzangy.video;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;

import android.app.Activity;
import android.content.Intent;
import android.database.Cursor;
import android.os.AsyncTask;
import android.os.Bundle;
import android.provider.MediaStore;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.Toast;

public class SearchVideosActivity extends Activity implements
		AdapterView.OnItemClickListener {
	private ListView vThumbnailsLv;
	private ArrayList<VideoInfo> vInfos;
	/**
	 * Called when the activity is first created.
	 */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		UiUtils.requestFullScreen(this);
		UiUtils.init(this);
		setContentView(R.layout.search_videos_layout);
		vThumbnailsLv = (ListView) findViewById(R.id.video_thumbnails_listview);
		vThumbnailsLv.setOnItemClickListener(this);
		 new SearchTask().execute();
	}

	private void scannerVideoToList() {
		// MediaStore.Video.Thumbnails.DATA:视频缩略图的文件路径 6.
		String[] thumbColumns = { MediaStore.Video.Thumbnails.DATA,
				MediaStore.Video.Thumbnails.VIDEO_ID };

		// MediaStore.Video.Media.DATA：视频文件路径； 10.
		// MediaStore.Video.Media.DISPLAY_NAME : 视频文件名，如 testVideo.mp4
		// MediaStore.Video.Media.TITLE: 视频标题 : testVideo
		String[] mediaColumns = { MediaStore.Video.Media._ID,
				MediaStore.Video.Media.DATA, MediaStore.Video.Media.TITLE,
				MediaStore.Video.Media.MIME_TYPE,
				MediaStore.Video.Media.DISPLAY_NAME,
				MediaStore.Video.Media.SIZE,
				MediaStore.Video.Media.DATE_MODIFIED};

		Cursor cursor = this.getContentResolver().query(MediaStore.Video.Media.EXTERNAL_CONTENT_URI,
				mediaColumns, null, null, null);

		if (cursor == null) {

			Toast.makeText(this, R.string.no_video, Toast.LENGTH_LONG).show();
			return;
		}
		vInfos = new ArrayList<VideoInfo>();
		if (cursor.moveToFirst()) {
			do {

				VideoInfo info = new VideoInfo();

				int id = cursor.getInt(cursor.getColumnIndex(MediaStore.Video.Media._ID));
				Cursor thumbCursor = this.getContentResolver().query(
						MediaStore.Video.Thumbnails.EXTERNAL_CONTENT_URI,
						thumbColumns, MediaStore.Video.Thumbnails.VIDEO_ID + "=" + id, null, null);
				if (thumbCursor.moveToFirst()) {
					info.setThumbnailPath(thumbCursor.getString(thumbCursor.getColumnIndex(MediaStore.Video.Thumbnails.DATA)));
				}
				info.setFilePath(cursor.getString(cursor.getColumnIndexOrThrow(MediaStore.Video.Media.DATA)));
				info.setTitle(cursor.getString(cursor.getColumnIndexOrThrow(MediaStore.Video.Media.TITLE)));
				info.setDisplayName(cursor.getString(cursor.getColumnIndexOrThrow(MediaStore.Video.Media.DISPLAY_NAME)));
				info.setMimeType(cursor.getString(cursor.getColumnIndexOrThrow(MediaStore.Video.Media.MIME_TYPE)));
				info.setSize(cursor.getInt(cursor.getColumnIndexOrThrow(MediaStore.Video.Media.SIZE)));
				info.setDateModified(cursor.getColumnIndexOrThrow(MediaStore.Video.Media.DATE_MODIFIED));
				Utils.logI("videoInfo=="+info.toString());
				vInfos.add(info);
			} while (cursor.moveToNext());
		}
	}

//	public static String[] getSdCardPaths(StorageManager strgMgr) {
//		Method method = null;
//		String[] paths = null;
//		try {
//			method = strgMgr.getClass().getMethod("getVolumePaths", null);
//			if (method != null) {
//				paths = (String[]) method.invoke(strgMgr, null);
//			}
//		} catch (NoSuchMethodException e) {
//			e.printStackTrace();
//		} catch (InvocationTargetException e) {
//			e.printStackTrace();
//		} catch (IllegalArgumentException e) {
//			e.printStackTrace();
//		} catch (IllegalAccessException e) {
//			e.printStackTrace();
//		}
//		if (paths == null) {
//			paths = new String[1];
//			paths[0] = Environment.getExternalStorageDirectory()
//					.getAbsolutePath();
//		}
//		return paths;
//	}
//
//	public static void scannerVideoFromFile(final List<VideoInfo> vList,
//			final File file) {
//		file.listFiles(new FileFilter() {
//			@Override
//			public boolean accept(File pathname) {
//				if (pathname.isDirectory() && !pathname.isHidden()) {
//					scannerVideoFromFile(vList, pathname);
//				} else {
//					String name = pathname.getName();
//					int index = name.indexOf(".");
//					if (index != -1) {
//						name = name.substring(index);
//						if (name.equalsIgnoreCase(".mp4")
//								|| name.equalsIgnoreCase(".3gp")
//								|| name.equalsIgnoreCase(".wmv")
//								|| name.equalsIgnoreCase(".ts")
//								|| name.equalsIgnoreCase(".rmvb")
//								|| name.equalsIgnoreCase(".mov")
//								|| name.equalsIgnoreCase(".m4v")
//								|| name.equalsIgnoreCase(".avi")
//								|| name.equalsIgnoreCase(".m3u8")
//								|| name.equalsIgnoreCase(".3gpp")
//								|| name.equalsIgnoreCase(".3gpp2")
//								|| name.equalsIgnoreCase(".mkv")
//								|| name.equalsIgnoreCase(".flv")
//								|| name.equalsIgnoreCase(".divx")
//								|| name.equalsIgnoreCase(".f4v")
//								|| name.equalsIgnoreCase(".rm")
//								|| name.equalsIgnoreCase(".asf")
//								|| name.equalsIgnoreCase(".ram")
//								|| name.equalsIgnoreCase(".mpg")
//								|| name.equalsIgnoreCase(".v8")
//								|| name.equalsIgnoreCase(".swf")
//								|| name.equalsIgnoreCase(".m2v")
//								|| name.equalsIgnoreCase(".asx")
//								|| name.equalsIgnoreCase(".ra")
//								|| name.equalsIgnoreCase(".ndivx")
//								|| name.equalsIgnoreCase(".xvid")) {
//							VideoInfo info = new VideoInfo();
//							info.filePath = pathname.getAbsolutePath();
//							info.title = pathname.getName();
//							info.thumbnailBmp = getVideoThumbnail(
//									info.filePath, 64, 64,
//									MediaStore.Images.Thumbnails.MICRO_KIND);
//							vList.add(info);
//						}
//					}
//					return true;
//				}
//				return false;
//			}
//		});
//	}
//
//	/**
//	 * 获取视频的缩略图 先通过ThumbnailUtils来创建一个视频的缩略图，然后再利用ThumbnailUtils来生成指定大小的缩略图。
//	 * 如果想要的缩略图的宽和高都小于MICRO_KIND，则类型要使用MICRO_KIND作为kind的值，这样会节省内存。
//	 * 
//	 * @param videoPath
//	 *            视频的路径
//	 * @param width
//	 *            指定输出视频缩略图的宽度
//	 * @param height
//	 *            指定输出视频缩略图的高度度
//	 * @param kind
//	 *            参照MediaStore.Images.Thumbnails类中的常量MINI_KIND和MICRO_KIND。
//	 *            其中，MINI_KIND: 512 x 384，MICRO_KIND: 96 x 96
//	 * @return 指定大小的视频缩略图
//	 */
//	public static Bitmap getVideoThumbnail(String videoPath, int width,
//			int height, int kind) {
//		Bitmap bitmap = null;
//		// 获取视频的缩略图
//		bitmap = ThumbnailUtils.createVideoThumbnail(videoPath, kind);
//		return bitmap;
//	}

	private class SearchTask extends AsyncTask<String, Integer, Long> {
		@Override
		protected Long doInBackground(String... paths) {
			long start = System.currentTimeMillis();
			scannerVideoToList();
			if(vInfos!=null){
				Collections.sort(vInfos, new Comparator<VideoInfo>(){
					@Override
					public int compare(VideoInfo arg0, VideoInfo arg1) {
						if(arg0.getDateModified()>arg1.getDateModified())
							return 1;
						else
							return -1;
					}
					
				});
			}
			int size = vInfos==null?0:vInfos.size();
			Utils.logI("scannerVideos Cost=="
			+ (System.currentTimeMillis() - start));
			return Long.valueOf(size);
		}

		@Override
		protected void onPreExecute() {
			super.onPreExecute();
		}

		@Override
		protected void onPostExecute(Long result) {
			super.onPostExecute(result);
			VideoThumbnailAdapter adapter = new VideoThumbnailAdapter(
					SearchVideosActivity.this, vInfos);
			vThumbnailsLv.setAdapter(adapter);
		}

		@Override
		protected void onProgressUpdate(Integer... values) {
			super.onProgressUpdate(values);
		}

		@Override
		protected void onCancelled(Long result) {
			super.onCancelled(result);
		}

		@Override
		protected void onCancelled() {
			super.onCancelled();
		}

	}

	@Override
	public void onItemClick(AdapterView<?> parent, View view, int position,
			long id) {
		if (vInfos != null && position < vInfos.size()) {
			Intent intent = new Intent(this, VideoActivity.class);
			intent.putExtra(VideoActivity.PLAY_VIDEO_INFO,
					vInfos.get(position).getFilePath());
			startActivity(intent);
		}
	}
}
