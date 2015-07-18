package com.angzangy.video;

import java.util.List;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.ThumbnailUtils;
import android.provider.MediaStore;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

public class VideoThumbnailAdapter extends BaseAdapter {
	private List<VideoInfo> infos;
	private Context ctxt;
	private LayoutInflater layoutInflater;
	private int thumbnailSize;

	public VideoThumbnailAdapter(Context context, List<VideoInfo> videoInfos) {
		infos = videoInfos;
		ctxt = context;
		layoutInflater = LayoutInflater.from(ctxt);
		thumbnailSize=UiUtils.screenHeight()/7;
	}

	@Override
	public int getCount() {
		if (infos != null) {
			return infos.size();
		}
		return 0;
	}

	@Override
	public Object getItem(int position) {
		return null;
	}

	@Override
	public long getItemId(int position) {
		return position;
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		if (getCount() > 0) {
			VideoInfo info = infos.get(position);
			if (convertView == null) {
				convertView = layoutInflater.inflate(R.layout.video_thumbnail_item_layout, null);
			}
			ImageView imgv = (ImageView) convertView.findViewById(R.id.thumbnail_imgv_id);
			TextView txtv = (TextView) convertView.findViewById(R.id.video_displayname_id);
			TextView sizetxtv = (TextView) convertView.findViewById(R.id.video_size_id);
			Bitmap thumbnail = createVoidoThumbnail(info,thumbnailSize,thumbnailSize);
			if (thumbnail != null) {
				imgv.setImageBitmap(thumbnail);
			}
			txtv.setText(info.getDisplayName());
			sizetxtv.setText(getVideoSize(info));
		}
		return convertView;
	}

	private Bitmap createVoidoThumbnail(VideoInfo info, int width, int height) {
		Bitmap bmp = null;
		if (info.getThumbnailPath() != null) {
			bmp = BitmapFactory.decodeFile(info.getThumbnailPath());
		} else {
			bmp = ThumbnailUtils.createVideoThumbnail(info.getFilePath(),
					MediaStore.Images.Thumbnails.MICRO_KIND);
		}
		if (bmp.getWidth() != width || bmp.getHeight() != height) {
			Bitmap tmp = ThumbnailUtils.extractThumbnail(bmp, width, height);
			bmp.recycle();
			bmp = tmp;
		}
		return bmp;
	}

	private String getVideoSize(VideoInfo info) {
		return info.getSize() / 1024.0f / 1024.0f + "MB";
	}
}
